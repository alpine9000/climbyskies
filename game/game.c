#include "game.h"
#include "version/version.h"

#ifdef SHOW_SPEED
#define SPEED_COLOR(x) custom->color[0] = x;
#else
#define SPEED_COLOR(x) 
#endif

#define GAME_LEVEL_BONUS_TRANSFER_RATE 32
#define GAME_RASTERAVERAGE_LENGTH 16

frame_buffer_t game_offScreenBuffer;
frame_buffer_t game_onScreenBuffer;
frame_buffer_t game_onScreenBuffer;
frame_buffer_t game_scoreBoardFrameBuffer;

int16_t game_cameraY;
int16_t game_screenScrollY;
int16_t game_scroll;
int16_t game_collisions;
int16_t game_numEnemies;
uint32_t game_levelScore;
uint32_t game_score;
uint32_t game_lives;
uint16_t game_level;

static void
game_switchFrameBuffers(void);
static void
game_newGame(menu_command_t command);
static void
game_loadLevel(menu_command_t command);
static void
game_render(void);
static 
void
game_scrollBackground(void);
static void
game_setCamera(int16_t offset);

static volatile __section(bss_c) uint8_t _frameBuffer1[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
static volatile __section(bss_c) uint8_t _bugBuffer1[FRAME_BUFFER_WIDTH_BYTES*1];
static volatile __section(bss_c) uint8_t _frameBuffer2[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
static volatile __section(bss_c) uint8_t _bugBuffer2[FRAME_BUFFER_WIDTH_BYTES*1];

uint16_t game_paused;
static uint16_t game_singleStep;
static uint32_t game_lastVerticalBlankCount;
static int16_t game_turtle;
#ifdef DEBUG
static uint16_t game_rasterLines[GAME_RASTERAVERAGE_LENGTH];
static uint16_t game_rasterLinesIndex = 0;
static uint16_t game_maxRasterLine = 0;
static uint16_t game_average = 0;
#endif
static int16_t game_targetCameraY;
static int16_t game_shake;
static int16_t game_scoreBoardMode;
static uint32_t game_lastScore;
static uint32_t game_lastLevelScore;
static uint16_t game_lastAverage;
static uint16_t game_lastMaxRasterLine;
static int16_t game_lastEnemyCount;
static int16_t game_lastItemCount;
static int16_t game_levelComplete;
static int16_t game_missedFrameCount;
static int16_t game_lastMissedFrameCount;

static void (*game_tileRender)(uint16_t hscroll);

static int16_t tileY;

 __section(data_c)  copper_t copper  = {
  .sprpt = {
    SPR0PTL,0x0000,
    SPR0PTH,0x0000,
    SPR1PTL,0x0000,
    SPR1PTH,0x0000,
    SPR2PTL,0x0000,
    SPR2PTH,0x0000,
    SPR3PTL,0x0000,
    SPR3PTH,0x0000,
    SPR4PTL,0x0000,
    SPR4PTH,0x0000,
    SPR5PTL,0x0000,
    SPR5PTH,0x0000,
    SPR6PTL,0x0000,
    SPR6PTH,0x0000,
    SPR7PTL,0x0000,
    SPR7PTH,0x0000    
  },
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
game_ctor(void)
{
  extern frame_buffer_t scoreBoardFrameBuffer;
  game_scoreBoardFrameBuffer = scoreBoardFrameBuffer;
  game_onScreenBuffer = (frame_buffer_t)&_frameBuffer1;
  game_offScreenBuffer = (frame_buffer_t)&_frameBuffer2;
}

__EXTERNAL void
game_init(menu_command_t command)
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

  screen_setup((uint16_t*)&copper);
  screen_pokeCopperList(game_scoreBoardFrameBuffer, copper.bpl3);
  
  game_newGame(command);
}

static void
debug_showScore(uint16_t refresh)
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


#ifdef DEBUG_SCROLL
  if (game_scoreBoardMode == 10) {
    text_drawScoreBoard(text_intToHex((uint32_t)tile_tilePtr, 8), 0);

    text_drawScoreBoard(text_intToAscii((uint32_t)tile_tilePtr, 8), 8*10);
    return;
  }
#endif

  if (game_scoreBoardMode == 0) {
    if (game_levelScore != game_lastLevelScore) {
      static char *buffer = "xxxxxxxxxx";
      char* score = text_intToAscii(game_levelScore>>2, 4);
      int x = 8*8;
      for (char* ptr = score, *btr = buffer; *ptr != 0; ptr++, btr++) {
	if (*ptr != *btr || refresh) {	  
	  text_drawCharScoreBoard(*ptr, x);
	  *btr = *ptr;
	}
	x+= 8;
      }
      game_lastLevelScore = game_levelScore;
    }
   
    if (game_score != game_lastScore) {
      static char *buffer = "yyyyyyyyyy";
      char* score = text_intToAscii(game_score, 6);
      int x = SCREEN_WIDTH-(7*8);
      for (char* ptr = score, *btr = buffer; *ptr != 0; ptr++, btr++) {
	if (*ptr != *btr || refresh) {	  
	  text_drawCharScoreBoard(*ptr, x);
	  *btr = *ptr;
	}
	x+= 8;
      }
      //   text_drawScoreBoard(text_intToAscii(game_score, 6), SCREEN_WIDTH-(6*8));
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

#ifdef GAME_RECORDING    
    switch (record_getState()) {
    case RECORD_IDLE:
      text_drawScoreBoard(" SCORE  " , SCREEN_WIDTH-(14*8));  
      break;
    case RECORD_RECORD:
      text_drawScoreBoard("RECORD  " , SCREEN_WIDTH-(14*8));        
      break;
    case RECORD_PLAYBACK:
#endif
      text_drawScoreBoard("REPLAY  " , SCREEN_WIDTH-(14*8));        
#ifdef GAME_RECORDING
      break;
    }
#endif

    text_drawScoreBoard("BONUS 0" , 8);  
    debug_showScore(1);
    uint32_t i, x;
    for (i = 0, x = (SCREEN_WIDTH/2)-17; i < game_lives; i++, x+=10) {
      gfx_renderSprite(game_scoreBoardFrameBuffer, 208, 176, x, 5, 16, 8);
    }
    
    for (; i < 3; i++, x+= 10) {
      gfx_renderSprite(game_scoreBoardFrameBuffer, 208, 184, x, 5, 16, 8);
    }
  }
}


void
game_loseLife(void)
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
  gfx_fillRect(game_scoreBoardFrameBuffer, 0, 0, FRAME_BUFFER_WIDTH, SCOREBOARD_HEIGHT, 0);
  game_lastAverage = -1;
  game_lastMaxRasterLine = -1;
  game_lastEnemyCount = -1;
  game_lastItemCount = -1;
  for (int16_t i = 0, x = (SCREEN_WIDTH/2)-15; i < 3; i++, x+=10) {
    gfx_renderSprite(game_scoreBoardFrameBuffer, 208, 184, x, 0, 16,  8);
  }

}

static void
game_newGame(menu_command_t command)
{
  game_score = 0;
  game_lives = 3;
  game_level = 0;

  if (command >= MENU_COMMAND_LEVEL) {
    game_level = command - MENU_COMMAND_LEVEL;
    command = MENU_COMMAND_PLAY;
  }

  game_loadLevel(command);
}

static void
game_loadLevel(menu_command_t command)
{  

  custom->bltafwm = 0xffff;
  game_turtle = 0;
#ifdef DEBUG
  game_average = 0;
  game_maxRasterLine = 0;
  game_rasterLinesIndex = 0;
#endif
  game_cameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
  hw_verticalBlankCount = 0;
  game_lastVerticalBlankCount = 0;
  game_singleStep = 0;
  game_paused = 0;
#ifdef DEBUG_SCROLL
  game_collisions = 0;
#else
  game_collisions = 1;
#endif
  game_screenScrollY = 0;
  game_shake = 0;
  game_setBackgroundScroll(SCROLL_PIXELS, WORLD_HEIGHT-SCREEN_HEIGHT);
  game_levelScore = 9999<<2;
#ifdef DEBUG_SCROLL
  game_scoreBoardMode = 10;
#else
  game_scoreBoardMode = 0;
#endif
  game_levelComplete = 0;
  game_lastScore = 1;
  game_lastLevelScore = 0;
  game_lastAverage = -1;
  game_lastMaxRasterLine = -1;
  game_lastEnemyCount = -1;
  game_lastItemCount = -1;
  game_lastMissedFrameCount = -1;
  game_missedFrameCount = 0;
  tileY = 0;

  game_switchFrameBuffers();

  sound_init();
  item_init(); // this must be initialised before tile
  enemy_init(); // this must be initialised before tile

  level_load(game_level);

  tile_init();
  tile_renderScreen();

  player_init(command);

  cloud_init();
  
  //gfx_fillRect(game_scoreBoardFrameBuffer, 0, 0, FRAME_BUFFER_WIDTH, SCOREBOARD_HEIGHT, 0);

  game_refreshScoreboard();
  //  text_drawScoreBoard(text_intToAscii(version, 4), SCREEN_WIDTH-(4*8));  

  hw_waitBlitter();

  game_render();

  game_switchFrameBuffers();

  game_render();

  hw_waitVerticalBlank();
  palette_fadeIn(level.fadeIn);
}

static inline void
game_switchFrameBuffers(void)
{
  uint16_t copperLine = RASTER_Y_START+game_screenScrollY;
  
  if (copperLine < 256) {
    copper.wait1[0] = (copperLine<<8)|1;
    copper.wait2[0] = (copperLine<<8)|1;
    copper.wait3[0] = 0xffdf;
    screen_pokeCopperList(game_offScreenBuffer+(int)gfx_dyOffsetsLUT[FRAME_BUFFER_HEIGHT-game_screenScrollY], copper.bpl1);
    screen_pokeCopperList(game_offScreenBuffer, copper.bpl2);
    screen_pokeCopperList(game_scoreBoardFrameBuffer, copper.bpl3);
  } else if (copperLine >= 256) {
    copper.wait1[0] = 0xffdf;
    if (game_screenScrollY >= 256) {
      copper.wait2[0] = (RASTER_Y_START)<<8|1;
      copper.wait3[0] = 0xffdf;
      screen_pokeCopperList(game_offScreenBuffer+(int)gfx_dyOffsetsLUT[FRAME_BUFFER_HEIGHT-game_screenScrollY], copper.bpl1);
      screen_pokeCopperList(game_scoreBoardFrameBuffer, copper.bpl2);
    } else {
      copper.wait2[0] = ((copperLine-256)<<8)|1;
      copper.wait3[0] = (RASTER_Y_START)<<8|1;
      screen_pokeCopperList(game_offScreenBuffer+(int)gfx_dyOffsetsLUT[FRAME_BUFFER_HEIGHT-game_screenScrollY], copper.bpl1);
      screen_pokeCopperList(game_offScreenBuffer, copper.bpl2);
      screen_pokeCopperList(game_scoreBoardFrameBuffer, copper.bpl3);
    }
  }


  frame_buffer_t save = game_onScreenBuffer;
  game_onScreenBuffer = game_offScreenBuffer;
  game_offScreenBuffer = save;
}

void 
game_shakeScreen(void)
{
  game_shake = 5;
}

static void
game_setCamera(int16_t offset)
{
  int16_t cameraSave = game_cameraY;
  game_cameraY -= offset;

  if (game_cameraY <= 0) {
    // this should not be possible
  } else if (game_cameraY >= WORLD_HEIGHT-SCREEN_HEIGHT) {
    game_cameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
    game_targetCameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
    game_scroll = cameraSave-game_cameraY;
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
  int16_t screenScrollSave = game_screenScrollY;
  game_setCamera(game_scroll);
  int16_t count = abs(game_scroll);

  gfx_setupRenderTileOffScreen();

  for (int s = 0, sy = screenScrollSave;  s < (count); s++) {
    if (game_scroll > 0) {
      sy++;
      tileY = (((sy-1) >> 4) << 4);
    } else {
      sy--;
      tileY = (((sy+1) >> 4) << 4);
    }

    (*game_tileRender)(tileY);
  }
}



#ifdef DEBUG
static void
debug_showRasterLine(void)
{  
  if (game_scoreBoardMode == 1) {
    static int16_t frame = 0;
    
    if (frame == 0) {
      if (game_average != game_lastAverage) {
	  text_drawScoreBoard(text_intToAscii(game_average, 4), 0);
	  game_lastAverage = game_average;
      }
    } else if(frame == 1){
      if (game_maxRasterLine != game_lastMaxRasterLine) {
	text_drawScoreBoard(text_intToAscii(game_maxRasterLine, 4), 5*8);
	game_lastMaxRasterLine = game_maxRasterLine;
      }
    } else if (frame == 2) {
      if (enemy_count != game_lastEnemyCount) {
	text_drawScoreBoard(text_intToAscii(enemy_count, 4), 10*8);
	game_lastEnemyCount = enemy_count;
      }
    } else if (frame == 3) {
      if (item_count != game_lastItemCount) {
	text_drawScoreBoard(text_intToAscii(item_count, 4), 15*8);
	game_lastItemCount = item_count;
      }
    } else if (frame == 4) {
      if (game_missedFrameCount != game_lastMissedFrameCount) {
	text_drawScoreBoard(text_intToAscii(game_missedFrameCount, 4), 20*8);
	game_lastMissedFrameCount = game_missedFrameCount;
      }
    }
    frame++;
    if (frame > 4) {
      frame = 0;
    }
  }


  
  int16_t line = hw_getRasterLine() - RASTER_Y_START;  

  if (line < 0) {
    line = 0;
  }

  game_rasterLines[game_rasterLinesIndex++] = line;
  if (line > game_maxRasterLine) {
    game_maxRasterLine = line;
  }
  if (game_rasterLinesIndex >= GAME_RASTERAVERAGE_LENGTH) {
    game_rasterLinesIndex = 0;
  }

  game_average = 0;
  for (int16_t i = 0; i < GAME_RASTERAVERAGE_LENGTH; i++) {
    game_average += game_rasterLines[i];
  }
  game_average = game_average >> 4 /* / GAME_RASTERAVERAGE_LENGTH */;
  
  return;  
}

#endif

static void
game_render(void)
{
  tile_renderInvalidTiles(game_offScreenBuffer);

  item_saveBackground(game_offScreenBuffer);

  enemy_saveBackground(game_offScreenBuffer);

  //#ifndef PLAYER_HARDWARE_SPRITE
  player_saveBackground(game_offScreenBuffer);
  //#endif

  if (level.clouds) {
    cloud_saveBackground(game_offScreenBuffer);
  }
  
  SPEED_COLOR(0x500);
  if (level.clouds) {
    cloud_render(game_offScreenBuffer);
  }
  SPEED_COLOR(0x202);
  item_render(game_offScreenBuffer);  
  SPEED_COLOR(0x050);
  enemy_render(game_offScreenBuffer);  
  SPEED_COLOR(0x005);
  player_render(game_offScreenBuffer);
}


void
game_setBackgroundScroll(int16_t s, int16_t targetCameraY)
{
  game_scroll = s;
  game_targetCameraY = targetCameraY;

  if (game_targetCameraY < 0) {
    game_targetCameraY = 0;
  } else if (game_targetCameraY > WORLD_HEIGHT-SCREEN_HEIGHT) {
    game_targetCameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
  }

  if (game_targetCameraY == game_cameraY) {
    game_scroll = 0;
  }

  if (game_scroll >= 0) {
    game_tileRender = tile_renderNextTile;
  } else {
    game_tileRender = tile_renderNextTileDown;
  }
}


void
game_setLevelComplete(void)
{
  if (!game_levelComplete) {
    text_drawText8(game_offScreenBuffer+(FRAME_BUFFER_WIDTH_BYTES*3), "LEVEL COMPLETE!", 8+(SCREEN_WIDTH/2)-(8*8), (SCREEN_HEIGHT/2)-64);
    text_drawText8(game_onScreenBuffer+(FRAME_BUFFER_WIDTH_BYTES*3), "LEVEL COMPLETE!", 8+(SCREEN_WIDTH/2)-(8*8), (SCREEN_HEIGHT/2)-64);
    game_levelComplete = 1;
    game_collisions = 0;
  }
}

static void
game_playLevel(uint16_t levelIndex)
{
  game_level = levelIndex;
  if (game_level >= LEVEL_NUM_LEVELS) {
    game_level = 0;
  }
  palette_black();
  game_loadLevel(MENU_COMMAND_PLAY);
#ifdef GAME_RECORDING
  record_setState(RECORD_IDLE);
  game_refreshScoreboard();
#endif
}

void
game_startPlayback(void)
{
  palette_black();
  game_loadLevel(MENU_COMMAND_REPLAY);
  game_refreshScoreboard();
}

void
game_startRecord(void)
{
  palette_black();
  game_loadLevel(MENU_COMMAND_REPLAY);
  game_refreshScoreboard();
  record_setState(RECORD_RECORD);
  game_refreshScoreboard();
}

int16_t
game_processKeyboard(void)
{
#ifdef GAME_RECORDING
  switch (keyboard_getKey() ) {
  case 'D':
    game_scoreBoardMode = !game_scoreBoardMode;
    if (game_scoreBoardMode == 0) {
      gfx_fillRect(game_scoreBoardFrameBuffer, 0, 0, FRAME_BUFFER_WIDTH, SCOREBOARD_HEIGHT, 0);
      game_refreshScoreboard();
    } else {
      game_refreshDebugScoreboard();
    }
    game_collisions = !game_collisions;
    break;
  case 'C':
    game_setLevelComplete();
    break;
  case 'T':
    game_singleStep = 1;
    break;
  case ' ':
    game_paused = !game_paused;
    break;
  case 'R':
    game_startRecord();
    break;
  case 'P':
    game_startPlayback();
    break;
  case 'S':
    record_setState(RECORD_IDLE);
    game_refreshScoreboard();
    break;
  case 'Z':
    music_next();
    break;
  case 'N':
    game_playLevel(game_level+1);
    break;
  case 'M':
    music_toggle_music();
    break;
#ifdef GAME_JETPACK
  case 'J':
    player.jetpackFuel += 100000;
    break;
#endif
  case 'Q':
    return 1;
    break;
  case '1':
    game_playLevel(0);
    break;
  case '2':
    game_playLevel(1);
    break;
  case '3':
    game_playLevel(2);
    break;
  }
#endif
  return 0;
}


__EXTERNAL void
game_loop()
{
  static int16_t operationCount = -1;
  int16_t joystickDown = 1;

  game_ctor();

  message_screenOn("Welcome to Climby Skies!");

  music_play(0);   

  hw_interruptsInit(); // Don't enable interrupts until music is set up

  music_toggle_music();

  menu_command_t menuCommand;
 menu:
  game_level = 0;
  if ((menuCommand = menu_loop()) == MENU_COMMAND_EXIT) {
#if TRACKLOADER==0
    goto done;
#endif
  }

  game_init(menuCommand);

  for (;;) {
#ifdef DEBUG
    if (game_paused && game_singleStep != 1) {
      goto skip;
    }
    game_singleStep = 0;
#endif

    hw_readJoystick();

    SPEED_COLOR(0xF0F);
    player_update();
    SPEED_COLOR(0x0fF);
    enemy_update(&player.sprite);
    SPEED_COLOR(0x2f2);
    item_update(&player.sprite);

    if (game_shake == 0) {
      if (level.clouds) {
	cloud_update();
      }
    } else if (game_scroll == 0) {
      game_shake--;
      if (game_shake > 0) {

	if (game_cameraY == WORLD_HEIGHT-SCREEN_HEIGHT) {
	  game_setBackgroundScroll(-2, game_cameraY - 12);
	} else {
	  game_setBackgroundScroll(2, game_cameraY + 12);
	}
	game_scroll = -game_scroll;
      }
    } 

    if (game_turtle > 1) {
      custom->color[16] = 0xf00;
      game_turtle--;
    } else if (game_turtle == 1) {
      custom->color[16] = 0x09e;
      game_turtle--;
    }

    if (hw_getRasterLine() < 220) {
      // if (operationCount == 10) {
	
	SPEED_COLOR(0xfff);
	debug_showScore(0);
	SPEED_COLOR(0x000);
	
	//	operationCount = 0;
	//      } 
      
#ifdef DEBUG
      debug_showRasterLine();
#endif
    }

    
    sound_schedule();
    hw_waitVerticalBlank();
#ifdef PLAYER_HARDWARE_SPRITE
    // this was before hw_waitVerticalBlank but caused glitches when things went too fast
    player_hSpriteRender();
#endif

    sound_vbl();

#ifdef PLAYER_HARDWARE_SPRITE
    player_updateCopper();
#endif
    
    if (game_lastVerticalBlankCount == 0) {

    } else if (hw_verticalBlankCount-game_lastVerticalBlankCount > 1) {
      game_missedFrameCount++;
      game_turtle = 5;
    }
      
    game_lastVerticalBlankCount = hw_verticalBlankCount;


    SPEED_COLOR(0xf00);
    game_switchFrameBuffers();

    if (game_cameraY != game_targetCameraY) {
      game_scrollBackground();
      if (game_cameraY == game_targetCameraY) {
	game_scroll = 0;
      }
    }


       


    SPEED_COLOR(0x0f0);
    enemy_restoreBackground();
    SPEED_COLOR(0xff0);
    item_restoreBackground();
    SPEED_COLOR(0xf00);
    //#ifndef PLAYER_HARDWARE_SPRITE
    player_restoreBackground();
    //#endif
    SPEED_COLOR(0x00f);
    if (level.clouds) {
      cloud_restoreBackground();
    }
    SPEED_COLOR(0x000);

    game_render();

#ifdef DEBUG
  skip:;
#endif

    if (!joystickDown && JOYSTICK_BUTTON_DOWN) {    
      joystickDown = 1;
      if (game_levelComplete && game_levelScore == 0) {
	game_playLevel(game_level + 1);
      }
    }

    joystickDown = JOYSTICK_BUTTON_DOWN;


    if (game_processKeyboard()) {
      goto menu;
    }
    operationCount++;
  }


#if TRACKLOADER==0
 done:;
#ifdef GAME_KEYBOARD_ENABLED
  keyboard_dtor();
#endif
#endif
}


#if 0
void *__memset(__REG("a0", void *dst), __REG("d0", int32_t c), __REG("d1", uint32_t n))
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


int
strlen(char* s) 
{
  int count = 0;
  while (*s++ != 0) {
    count++;
  }

  return count;
}
