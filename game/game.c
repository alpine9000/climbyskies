#include "game.h"
#include "version/version.h"

#ifdef SHOW_SPEED
#define SPEED_COLOR(x) custom->color[0] = x;
#else
#define SPEED_COLOR(x) 
#endif

#define GAME_LEVEL_BONUS_TRANSFER_RATE 32

frame_buffer_t game_offScreenBuffer;
frame_buffer_t game_onScreenBuffer;
frame_buffer_t game_saveBuffer;
frame_buffer_t game_scoreBoardFrameBuffer;

int game_cameraY;
int game_screenScrollY;
int game_scrollCount;
int game_scroll;
int game_levelComplete;
#ifdef GAME_PAUSE_DISABLES_COLLISION
int game_paused;
#endif
int game_numEnemies;
uint32_t game_levelScore;
uint32_t game_score;
uint32_t game_lives;

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
static frame_buffer_t saveBuffer1;
static frame_buffer_t saveBuffer2;
static uint32_t lastVerticalBlankCount;
static int turtle;

#define AVERAGE_LENGTH 16
static int rasterLines[AVERAGE_LENGTH];
static int rasterLinesIndex = 0;
static int maxRasterLine = 0;
static int average = 0;
static int game_shake;
static int game_scoreBoardMode;
static uint32_t game_lastScore;
static uint32_t game_lastLevelScore;
static int game_lastAverage;
static int game_lastMaxRasterLine;
static int game_lastEnemyCount;






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

  game_score = 0;
  game_lives = 3;
  game_onScreenBuffer = (frame_buffer_t)&_frameBuffer1;
  game_scoreBoardFrameBuffer = (frame_buffer_t)&_scoreBoardBuffer;
  game_offScreenBuffer = (frame_buffer_t)&_frameBuffer2;
  game_saveBuffer = (frame_buffer_t)&_saveBuffer1;
  saveBuffer1 = (frame_buffer_t)&_saveBuffer1;
  saveBuffer2 = (frame_buffer_t)&_saveBuffer2;
  screen_setup((uint16_t*)&copper);
  screen_pokeCopperList(game_scoreBoardFrameBuffer, copper.bpl3);

  music_play(0);   
  hw_interruptsInit(); // Don't enable interrupts until music is set up

  game_newGame();
}

static void
debug_showScore(void)
{
  if (game_levelComplete) {
    if (game_levelScore >= GAME_LEVEL_BONUS_TRANSFER_RATE<<2) {
      game_score+=GAME_LEVEL_BONUS_TRANSFER_RATE;
      game_levelScore-=(GAME_LEVEL_BONUS_TRANSFER_RATE<<2);
    } else if (game_levelScore > 0){
      game_score += game_levelScore>>2;
      game_levelScore = 0;
    }
  } else {
    game_levelScore--;
  }

  if (game_scoreBoardMode == 0) {
    if (game_levelScore != game_lastLevelScore) {
      text_drawScoreBoard(text_intToAscii(game_levelScore>>2, 4), 8*8);
      game_lastLevelScore = game_levelScore;
    }
    
    if (game_score != game_lastScore) {
      text_drawScoreBoard(text_intToAscii(game_score, 6), SCREEN_WIDTH-(6*8));
      game_lastScore = game_score;
    }
  }
}

static void
game_refreshScoreboard(void)
{
  game_lastScore = 1;
  game_lastLevelScore = 0;

  if (game_scoreBoardMode == 0) {
    text_drawScoreBoard("SCORE  " , SCREEN_WIDTH-(12*8));  
    text_drawScoreBoard("BONUS 00      " , 0);  
    debug_showScore();
    uint32_t i, y;
    for (i = 0, y = (SCREEN_WIDTH/2)-15; i < game_lives; i++, y+=10) {
      gfx_renderSprite(game_scoreBoardFrameBuffer, 208, 176, y, 0, 16,  8);
    }
    
    for (; i < 3; i++) {
      gfx_renderSprite(game_scoreBoardFrameBuffer, 208, 184, y, 0, 16,  8);
    }
  }
}


void
game_loseLife()
{ 
  if (game_lives > 0) {
    game_lives--;
    game_refreshScoreboard();
  } else {
    // game over
  }
}

static 
void
game_refreshDebugScoreboard(void)
{
  game_lastAverage = -1;
  game_lastMaxRasterLine = -1;
  game_lastEnemyCount = -1;
  text_drawScoreBoard("            " , SCREEN_WIDTH-(12*8));  
  text_drawScoreBoard("            " , 0);  
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
#ifdef GAME_PAUSE_DISABLES_COLLISION
  game_paused = 1;
#endif
  game_screenScrollY = 0;
  game_scrollCount = 0;
  game_shake = 0;
  game_setBackgroundScroll(SCROLL_PIXELS);
  game_levelScore = 9999<<2;
  game_scoreBoardMode = 1;
  game_levelComplete = 0;
  game_lastScore = 1;
  game_lastLevelScore = 0;
  game_lastAverage = -1;
  game_lastMaxRasterLine = -1;
  game_lastEnemyCount = -1;
  tileY = 0;

  game_switchFrameBuffers();

  item_init(); // this must be initialised before tile
  enemy_init(); // this must be initialised before tile

  tile_init();
  tile_renderScreen();

  player_init();

  cloud_init();
  
  gfx_fillRect(game_scoreBoardFrameBuffer, 0, 0, FRAME_BUFFER_WIDTH, SCOREBOARD_HEIGHT, 0);

  game_refreshScoreboard();
  //  text_drawScoreBoard(text_intToAscii(version, 4), SCREEN_WIDTH-(4*8));  

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

void 
game_shakeScreen(void)
{
  game_shake = 6;
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

  gfx_setupRenderTileOffScreen();

  for (int s = 0;  s < count && tileIndex+s < SCREEN_WIDTH/TILE_HEIGHT; s++) {
    (*game_tileRender)(tileY);
  }

  if (tileIndex == 0) {	 
     tileY = game_screenScrollY;
  }
}



static void
debug_showRasterLine(void)
{
  if (turtle > 1) {
    custom->color[1] = 0xf00;
    turtle--;
  } else if (turtle == 1) {
    custom->color[1] = 0x09e;
    turtle--;
  }
  

  if (game_scoreBoardMode == 1 && hw_getRasterLine() < 250) {
    static int frame = 0;
    
    if (frame == 0) {
      if (average != game_lastAverage) {
	text_drawScoreBoard(text_intToAscii(average, 4), 0);
	game_lastAverage = average;
      }
    } else if( frame == 1){
      if (maxRasterLine != game_lastMaxRasterLine) {
	text_drawScoreBoard(text_intToAscii(maxRasterLine, 4), 5*8);
	game_lastMaxRasterLine = maxRasterLine;
      }
    } else if (frame == 2) {
      if (enemy_count != game_lastEnemyCount) {
	text_drawScoreBoard(text_intToAscii(enemy_count, 4), 10*8);
	game_lastEnemyCount = enemy_count;
      }
    }
    frame++;
    if (frame > 2) {
      frame = 0;
    }
  }
  
  
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

  item_saveBackground(game_offScreenBuffer);

  enemy_saveBackground(game_offScreenBuffer);

  player_saveBackground(game_offScreenBuffer);

  cloud_saveBackground(game_offScreenBuffer);


  
  SPEED_COLOR(0x500);
  cloud_render(game_offScreenBuffer);
  SPEED_COLOR(0x202);
  item_render(game_offScreenBuffer);  
  SPEED_COLOR(0x050);
  enemy_render(game_offScreenBuffer);  
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

    if (!joystickDown && JOYSTICK_BUTTON_DOWN) {    
      joystickDown = 1;
      game_scoreBoardMode = !game_scoreBoardMode;
      if (game_scoreBoardMode == 0) {
	game_refreshScoreboard();
      } else {
	game_refreshDebugScoreboard();
      }
#ifdef GAME_PAUSE_DISABLES_COLLISION
      game_paused = !game_paused;
#endif

    }

    joystickDown = JOYSTICK_BUTTON_DOWN;

    //    if (!game_paused) {
    SPEED_COLOR(0xF0F);
    player_update();
    SPEED_COLOR(0x0fF);
    enemy_update(&player.sprite);
    SPEED_COLOR(0x2f2);
    item_update(&player.sprite);
    //    }


    if (game_shake == 0) {
      cloud_update();
    } else if (game_scrollCount == 0/* && game_shake > 0*/) {
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
    debug_showScore();
    SPEED_COLOR(0x000);


    hw_waitVerticalBlank();

    if (lastVerticalBlankCount == 0) {

    } else if (hw_verticalBlankCount-lastVerticalBlankCount > 1) {
      turtle = 5;
    }
      
    lastVerticalBlankCount = hw_verticalBlankCount;

    SPEED_COLOR(0xf00);
    game_switchFrameBuffers();

    if (game_scrollCount > 0 && game_scroll != 0) {
      game_scrollBackground();
      game_scrollCount--;
    }


    SPEED_COLOR(0x0f0);
    enemy_restoreBackground();
    SPEED_COLOR(0xff0);
    item_restoreBackground();
    SPEED_COLOR(0xf00);
    player_restoreBackground();
    SPEED_COLOR(0x00f);
    cloud_restoreBackground();

    SPEED_COLOR(0x000);

    game_render();

    if (game_levelComplete) {
      text_drawText8(game_offScreenBuffer+(FRAME_BUFFER_WIDTH_BYTES*3), "LEVEL COMPLETE!", 8+(SCREEN_WIDTH/2)-(8*8), (SCREEN_HEIGHT/2)-64);
    }


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
