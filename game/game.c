#include "game.h"
#include "version/version.h"

#ifdef SHOW_SPEED
#define SPEED_COLOR(x) custom->color[0] = x;
#else
#define SPEED_COLOR(x) 
#endif

frame_buffer_t game_offScreenBuffer;
frame_buffer_t game_onScreenBuffer;
frame_buffer_t game_saveBuffer;

int game_cameraY;
int game_screenScrollY;
int game_scrollCount;
int game_scroll;
static int game_shake;

static void
game_switchFrameBuffers(void);
static void
game_newGame(void);
static void
game_render(void);
static void
game_scrollBackground(void);
static void
game_setCamera(int offset);

static volatile __section(bss_c) uint8_t _frameBuffer1[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
static volatile __section(bss_c) uint8_t _saveBuffer1[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
static volatile __section(bss_c) uint8_t _frameBuffer2[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
static volatile __section(bss_c) uint8_t _saveBuffer2[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
static volatile __section(bss_c) uint8_t _scoreBoardBuffer[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(SCOREBOARD_HEIGHT)];
static frame_buffer_t scoreBoardFrameBuffer;
static frame_buffer_t saveBuffer1;
static frame_buffer_t saveBuffer2;
static uint32_t lastVerticalBlankCount;
static int turtle;

#define AVERAGE_LENGTH 16
static int rasterLines[AVERAGE_LENGTH];
static int rasterLinesIndex = 0;
static int maxRasterLine = 0;
static int average = 0;

static void (*game_tileRender)(uint16_t hscroll);

static int tileY;
static __section(data_c)  copper_t copper  = {

  .bpl1 = {
    BPL1PTL,0x0000,
    BPL1PTH,0x0000,
    BPL2PTL,0x0000,
    BPL2PTH,0x0000,
    BPL3PTL,0x0000,
    BPL3PTH,0x0000,
    BPL4PTL,0x0000,
    BPL4PTH,0x0000,
    BPL5PTL,0x0000,
    BPL5PTH,0x0000,
  },
  .wait1 = { 
    0xffdf,0xfffe 
  },
  .wait2 = { 
    (RASTER_Y_START-1)<<8|1,0xfffe
  },
  .bpl2= {
    BPL1PTL,0x0000,
    BPL1PTH,0x0000,
    BPL2PTL,0x0000,
    BPL2PTH,0x0000,
    BPL3PTL,0x0000,
    BPL3PTH,0x0000,
    BPL4PTL,0x0000,
    BPL4PTH,0x0000,
    BPL5PTL,0x0000,
    BPL5PTH,0x0000,
  },
   .wait3 = {     
    (RASTER_Y_START)<<8|1, 0xfffe,
    (RASTER_Y_START)<<8|1, 0xfffe,
    },
  .bpl3= {
    BPL1PTL,0x0000,
    BPL1PTH,0x0000,
    BPL2PTL,0x0000,
    BPL2PTH,0x0000,
    BPL3PTL,0x0000,
    BPL3PTH,0x0000,
    BPL4PTL,0x0000,
    BPL4PTH,0x0000,
    BPL5PTL,0x0000,
    BPL5PTH,0x0000,
  },
  .end = {0xFFFF, 0xFFFE}

};

__EXTERNAL void
game_init()
{
#if TRACKLOADER==1
  extern char* startBSS;
  extern char* endBSS;

  char* ptr = startBSS;

  while (ptr != endBSS) {
    *ptr++ = 0;
  }
#endif

  hw_waitVerticalBlank();
  palette_black();

  game_onScreenBuffer = (frame_buffer_t)&_frameBuffer1;
  scoreBoardFrameBuffer = (frame_buffer_t)&_scoreBoardBuffer;
  game_offScreenBuffer = (frame_buffer_t)&_frameBuffer2;
  game_saveBuffer = (frame_buffer_t)&_saveBuffer1;
  saveBuffer1 = (frame_buffer_t)&_saveBuffer1;
  saveBuffer2 = (frame_buffer_t)&_saveBuffer2;
  screen_setup((uint16_t*)&copper);
  screen_pokeCopperList(scoreBoardFrameBuffer, copper.bpl3);

  music_play(0);   
  hw_interruptsInit(); // Don't enable interrupts until music is set up

  game_newGame();
}


static void
game_newGame(void)
{
  turtle = 0;
  average = 0;
  maxRasterLine = 0;
  rasterLinesIndex = 0;
  game_cameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
  hw_verticalBlankCount = 0;
  lastVerticalBlankCount = 0;
  game_screenScrollY = 0;
  game_scrollCount = 0;
  game_shake = 0;
  game_setBackgroundScroll(SCROLL_PIXELS);
  tileY = 0;

  game_switchFrameBuffers();

  tile_init();
  tile_renderScreen();

  player_init();
#ifdef ENABLE_ENEMIES
  enemy_init();
#endif
  cloud_init();
  
  gfx_fillRect(scoreBoardFrameBuffer, 0, 0, FRAME_BUFFER_WIDTH, SCOREBOARD_HEIGHT, 0);

  text_drawText8(scoreBoardFrameBuffer, text_intToAscii(version, 4), SCREEN_WIDTH-(4*8), 4);  

  hw_waitBlitter();

  game_render();

  game_switchFrameBuffers();

  game_render();

  hw_waitVerticalBlank();
  palette_fadeIn();
}

static void
game_switchFrameBuffers(void)
{
  uint16_t copperLine = RASTER_Y_START+game_screenScrollY;
  
  if (copperLine < 256) {
    copper.wait1[0] = (copperLine<<8)|1;
    copper.wait2[0] = (copperLine<<8)|1;
    copper.wait3[0] = 0xffdf;
  } else if (copperLine >= 256) {
    copper.wait1[0] = 0xffdf;
    if (game_screenScrollY >= 256) {
      copper.wait2[0] = (RASTER_Y_START-1)<<8|1;
    } else {
      copper.wait2[0] = ((copperLine-256)<<8)|1;
    }
    copper.wait3[0] = (RASTER_Y_START)<<8|1;
  }

  screen_pokeCopperList(game_offScreenBuffer+(int)gfx_dyOffsetsLUT[FRAME_BUFFER_HEIGHT-game_screenScrollY], copper.bpl1);
  screen_pokeCopperList(game_offScreenBuffer, copper.bpl2);

  frame_buffer_t save = game_onScreenBuffer;
  game_onScreenBuffer = game_offScreenBuffer;
  game_offScreenBuffer = save;
}

#if 0
//static 
void
 game_shakeScroll(int direction, int count)
{
  int delay = 100;
  for (int c = 0; c < count; c++) {
    hw_waitScanLines(delay);
    hw_waitVerticalBlank();
    game_switchFrameBuffers();
    game_setCamera(direction);
    player_restoreBackground();
#ifdef ENABLE_ENEMIES
    enemy_restoreBackground();
#endif
    cloud_restoreBackground();
    game_render();
  }
  hw_waitScanLines(delay);
}
#endif

void 
game_shakeScreen(void)
{
  game_shake = 4;
  game_scrollCount = 0;  
  game_setBackgroundScroll(-2);
}

static void
game_setCamera(int offset)
{
  game_cameraY -= offset;

  if (game_cameraY <= 0 && game_scrollCount == 0) {
    game_cameraY = 0;
    game_scroll = 0;
    game_scrollCount = 0;
    return;
  } else if (game_cameraY > WORLD_HEIGHT-SCREEN_HEIGHT) {
    game_cameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
    game_scroll = 0;
    game_scrollCount = 0;
    return;
  }
 
#if 1
  game_screenScrollY = -((game_cameraY-(WORLD_HEIGHT-SCREEN_HEIGHT)) % FRAME_BUFFER_HEIGHT);
#else
  game_screenScrollY = -((game_cameraY-(WORLD_HEIGHT-SCREEN_HEIGHT)));

  while (game_screenScrollY >= FRAME_BUFFER_HEIGHT) {
    game_screenScrollY -= FRAME_BUFFER_HEIGHT;
  }
#endif
}

static void
game_scrollBackground(void)
{
  game_setCamera(game_scroll);

  int tileIndex = game_screenScrollY % TILE_HEIGHT;
  int count = abs(game_scroll);

  for (int s = 0;  s < count && tileIndex+s < SCREEN_WIDTH/TILE_HEIGHT; s++) {
    (*game_tileRender)(tileY);
  }

  if (tileIndex == 0) {	 
     tileY = game_screenScrollY;
  }
}

//static 
void
debug_showRasterLine(void)
{
#if 1
  if (turtle > 1) {
    custom->color[1] = 0xf00;
    //gfx_fillRect(scoreBoardFrameBuffer, 10*8, 0, 16, 16, 28);
    //    text_drawText8(scoreBoardFrameBuffer, "SLOW", 10*8, 4);  
    turtle--;
  } else if (turtle == 1) {
    custom->color[1] = 0x09e;
    //gfx_fillRect(scoreBoardFrameBuffer, 10*8, 0, 16, 16, 0);
    //    text_drawText8(scoreBoardFrameBuffer, "    ", 10*8, 4);  
    turtle--;
  }
#endif

  return;

  text_drawText8(scoreBoardFrameBuffer, text_intToAscii(average, 4), 0, 4);  
  text_drawText8(scoreBoardFrameBuffer, text_intToAscii(maxRasterLine, 4), 5*8, 4);

  int line = hw_getRasterLine() - RASTER_Y_START;  

  rasterLines[rasterLinesIndex++] = line;
  if (line > maxRasterLine) {
    maxRasterLine = line;
  }
  if (rasterLinesIndex >= AVERAGE_LENGTH) {
    rasterLinesIndex = 0;
  }

  average = 0;
  for (int i = 0; i < AVERAGE_LENGTH; i++) {
    average += rasterLines[i];
  }
  average = average >> 4 /* / AVERAGE_LENGTH */;
  


  
}

static void
game_render(void)
{
  tile_renderInvalidTiles(game_offScreenBuffer);

#ifdef ENABLE_ENEMIES
  enemy_saveBackground(game_offScreenBuffer);
#endif
  player_saveBackground(game_offScreenBuffer);

  cloud_saveBackground(game_offScreenBuffer);


  
  SPEED_COLOR(0x500);
  cloud_render(game_offScreenBuffer);
#ifdef ENABLE_ENEMIES
  SPEED_COLOR(0x050);
  enemy_render(game_offScreenBuffer);  
#endif
  SPEED_COLOR(0x005);
  player_render(game_offScreenBuffer);
  game_saveBuffer = game_saveBuffer == saveBuffer1 ? saveBuffer2 : saveBuffer1;
}

void
game_setBackgroundScroll(int s)
{
  game_scroll = s;
  if (game_scroll >= 0) {
    game_tileRender = tile_renderNextTile;
  } else {
    game_tileRender = tile_renderNextTileDown;
  }
}

__EXTERNAL void
game_loop()
{
  //  static int lastJoystickPos = 0;
  int done = 0;
  int joystickDown = 1;

  while (!done) {
    hw_readJoystick();

    if (game_scrollCount == 0 && !joystickDown && JOYSTICK_BUTTON_DOWN) {    
      //  game_scrollCount = 1;//1+((6*16)/SCROLL_PIXELS);
      //      game_scrollCount = 1000;
      //      game_setBackgroundScroll(-scroll);
      joystickDown = 1;
    }

    /*    if (hw_joystickPos == 5 && lastJoystickPos != 5) {
      game_scrollCount = 16;
      }*/

    //lastJoystickPos = hw_joystickPos;

    joystickDown = JOYSTICK_BUTTON_DOWN;

    SPEED_COLOR(0xF0F);
    player_update();
    SPEED_COLOR(0x0fF);
#ifdef ENABLE_ENEMIES
    enemy_update(&player.sprite);
#endif


    if (game_shake == 0) {
      cloud_update();
    }

    if (game_scrollCount == 0 && game_shake > 0) {
      game_shake--;
      if (game_shake > 1) {
	game_setBackgroundScroll(game_scroll);
	game_scroll = -game_scroll;
	game_scrollCount = 6;
      } else {
	game_setBackgroundScroll(-game_scroll);
      }
    } 


    SPEED_COLOR(0x00);


    SPEED_COLOR(0xfff);
    debug_showRasterLine();
    SPEED_COLOR(0x000);


    hw_waitVerticalBlank();

    if (lastVerticalBlankCount == 0) {

    } else if (hw_verticalBlankCount-lastVerticalBlankCount > 1) {
      turtle = 100;
    }
      
    lastVerticalBlankCount = hw_verticalBlankCount;

    SPEED_COLOR(0xf00);
    game_switchFrameBuffers();

    if (game_scrollCount > 0 && game_scroll != 0) {
      game_scrollBackground();
      game_scrollCount--;
    }


#ifdef ENABLE_ENEMIES
    SPEED_COLOR(0x0f0);
    enemy_restoreBackground();
#endif
    SPEED_COLOR(0xf00);
    //    text_restore();
    player_restoreBackground();
    SPEED_COLOR(0x00f);
    cloud_restoreBackground();

    SPEED_COLOR(0x000);

    game_render();


    //    hw_waitBlitter();

#if TRACKLOADER==0
    done = mouse_leftButtonPressed();
#endif
    if (mouse_leftButtonPressed()) {
      while (mouse_leftButtonPressed());
      palette_black();
      game_newGame();
      
    }
  }
}

#if 0
void *__memset(__REG("a0", void *dst), __REG("d0", int c), __REG("d1", uint32_t n))
{
  if (n) {
    char *d = dst;
    
    do {
      *d++ = c;
    } while (--n);
  }
  return dst;
}
#endif

#if defined(__GNUC__) && !defined(GCC_CHECK)
void* memcpy(void* destination, void* source, size_t num)
{
  size_t i;
  char* d = destination;
  char* s = source;
  for (i = 0; i < num; i++) {
    d[i] = s[i];
  }
  return destination;
}
#endif
