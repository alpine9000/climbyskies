#include "game.h"
#include "version/version.h"

//#define SHOW_SPEED 1

#ifdef SHOW_SPEED
#define SPEED_COLOR(x) custom->color[0] = x;
#else
#define SPEED_COLOR(x) 
#endif

frame_buffer_t offScreenBuffer;
frame_buffer_t onScreenBuffer;
frame_buffer_t saveBuffer;

int cameraY;
int screenScrollY;
char evil;
int scrollCount;
int scroll;
uint32_t frameCount;

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

void
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

  onScreenBuffer = (frame_buffer_t)&_frameBuffer1;
  scoreBoardFrameBuffer = (frame_buffer_t)&_scoreBoardBuffer;
  offScreenBuffer = (frame_buffer_t)&_frameBuffer2;
  saveBuffer = (frame_buffer_t)&_saveBuffer1;
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
  cameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
  verticalBlankCount = 0;
  lastVerticalBlankCount = 0;
  screenScrollY = 0;
  scrollCount = 0;
  frameCount = 0;
  game_setBackgroundScroll(SCROLL_PIXELS);
  tileY = 0;

  game_switchFrameBuffers();

  tile_init();
  tile_renderScreen();

#if 0
  palette_fadeIn();
  custom->color[1] = 0xf00;
  custom->color[0] = 0x00f;
  gfx_fillRect(offScreenBuffer, 0, 0, 100, 100, 1);
  gfx_renderSprite(offScreenBuffer, 0, 0, 0, 0, 64, 64);
  game_switchFrameBuffers();
  for (;;);
#endif

  player_init();
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
  uint16_t copperLine = RASTER_Y_START+screenScrollY;
  
  if (copperLine < 256) {
    copper.wait1[0] = (copperLine<<8)|1;
    copper.wait2[0] = (copperLine<<8)|1;
    copper.wait3[0] = 0xffdf;
  } else if (copperLine >= 256) {
    copper.wait1[0] = 0xffdf;
    if (screenScrollY >= 256) {
      copper.wait2[0] = (RASTER_Y_START-1)<<8|1;
    } else {
      copper.wait2[0] = ((copperLine-256)<<8)|1;
    }
    copper.wait3[0] = (RASTER_Y_START)<<8|1;
  }

  screen_pokeCopperList(offScreenBuffer+(int)dyOffsetsLUT[FRAME_BUFFER_HEIGHT-screenScrollY], copper.bpl1);
  screen_pokeCopperList(offScreenBuffer, copper.bpl2);

  frame_buffer_t save = onScreenBuffer;
  onScreenBuffer = offScreenBuffer;
  offScreenBuffer = save;
}

static 
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
    cloud_restoreBackground();
    game_render();
  }
  hw_waitScanLines(delay);
}

void 
game_shakeScreen(void)
{
  int direction = 1;
  for (int i = 0; i < 4; i++) {
    game_shakeScroll(direction, SCROLL_PIXELS);
    direction = -direction;
  }
}

static void
game_setCamera(int offset)
{
  cameraY -= offset;

  if (cameraY < 0) {
    cameraY = 0;
    scroll = 0;
    scrollCount = 0;
    return;
  } else if (cameraY > WORLD_HEIGHT-SCREEN_HEIGHT) {
    cameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
    scroll = 0;
    scrollCount = 0;
    return;
  }
 
#if 0
  screenScrollY = -((cameraY-(WORLD_HEIGHT-SCREEN_HEIGHT)) % FRAME_BUFFER_HEIGHT);
#else
  screenScrollY = -((cameraY-(WORLD_HEIGHT-SCREEN_HEIGHT)));

  while (screenScrollY >= FRAME_BUFFER_HEIGHT) {
    screenScrollY -= FRAME_BUFFER_HEIGHT;
  }
#endif
}

static void
game_scrollBackground(void)
{
  game_setCamera(scroll);

#if 0  
  int tileIndex = screenScrollY % TILE_HEIGHT;
#else
  int tileIndex = screenScrollY;
  
  while (tileIndex >= TILE_HEIGHT) {
    tileIndex -= TILE_HEIGHT;
  }
#endif


  int count = abs(scroll);

  for (int s = 0;  s < count && tileIndex+s < SCREEN_WIDTH/TILE_HEIGHT; s++) {
    (*game_tileRender)(tileY);
  }

  if (tileIndex == 0) {	 
     tileY = screenScrollY;
  }
}

static void
debug_showRasterLine(void)
{

  if (turtle > 1) {
    gfx_fillRect(scoreBoardFrameBuffer, 10*8, 0, 16, 16, 28);
    turtle--;
  } else if (turtle == 1) {
    gfx_fillRect(scoreBoardFrameBuffer, 10*8, 0, 16, 16, 0);
    turtle--;
  }


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
  tile_renderInvalidTiles(offScreenBuffer);

  player_saveBackground(offScreenBuffer);
  cloud_saveBackground(offScreenBuffer);


  
  SPEED_COLOR(0x0f0);
  cloud_render(offScreenBuffer);
  SPEED_COLOR(0x0ff);
  player_render(offScreenBuffer);
  saveBuffer = saveBuffer == saveBuffer1 ? saveBuffer2 : saveBuffer1;
}

void
game_setBackgroundScroll(int s)
{
  scroll = s;
  if (scroll >= 0) {
    game_tileRender = tile_renderNextTile;
  } else {
    game_tileRender = tile_renderNextTileDown;
  }
}

void
game_loop()
{
  //  static int lastJoystickPos = 0;
  int done = 0;
  int joystickDown = 1;

  while (!done) {
    frameCount++;
    hw_readJoystick();

    if (scrollCount == 0 && !joystickDown && JOYSTICK_BUTTON_DOWN) {    
      //  scrollCount = 1;//1+((6*16)/SCROLL_PIXELS);
      //      scrollCount = 1000;
      //      game_setBackgroundScroll(-scroll);
      joystickDown = 1;
    }

    /*    if (hw_joystickPos == 5 && lastJoystickPos != 5) {
      scrollCount = 16;
      }*/

    //lastJoystickPos = hw_joystickPos;

    joystickDown = JOYSTICK_BUTTON_DOWN;

    SPEED_COLOR(0xF0F);
    player_update();
    SPEED_COLOR(0x0fF);
    cloud_update();
    SPEED_COLOR(0x00);


    SPEED_COLOR(0xfff);
    debug_showRasterLine();
    SPEED_COLOR(0x000);


    hw_waitVerticalBlank();

    if (lastVerticalBlankCount == 0) {

    } else if (verticalBlankCount-lastVerticalBlankCount > 1) {
      turtle = 100;
    }
      
    lastVerticalBlankCount = verticalBlankCount;

    SPEED_COLOR(0xf00);
    game_switchFrameBuffers();

    if (scrollCount > 0 && scroll != 0) {
      game_scrollBackground();
      scrollCount--;
    }

    //    text_restore();
    player_restoreBackground();
    cloud_restoreBackground();

    game_render();

    //       for (volatile int i = 0; i < 1000; i++);

    hw_waitBlitter();

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


#if defined(__GNUC__) && !defined(GCC_CHECK)
void* memcpy(void* destination, void* source, int num)
{
  int i;
  char* d = destination;
  char* s = source;
  for (i = 0; i < num; i++) {
    d[i] = s[i];
  }
  return destination;
}
#endif
