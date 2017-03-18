#include "game.h"

#define GAME_SCORE_RASTERLINE_CUTOFF 330

#define GAME_LEVEL_BONUS_TRANSFER_RATE 32
#ifdef DEBUG
#define GAME_RASTERAVERAGE_LENGTH 16
#endif

static void
game_switchFrameBuffers(void);
static void
game_newGame(menu_command_t command);
static void
game_loadLevel(menu_command_t command);
static void
game_render(void);
static void
game_scrollBackground(void);

frame_buffer_t game_offScreenBuffer;
frame_buffer_t game_onScreenBuffer;
frame_buffer_t game_onScreenBuffer;

int16_t game_cameraY;
int16_t game_screenScrollY;
int16_t game_collisions;
uint32_t game_levelScore;
uint32_t game_score;
uint32_t game_lives;
uint16_t game_level;
uint16_t game_over;
uint16_t game_levelComplete;

static volatile __section(random_c) uint8_t _frameBuffer1[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
static volatile __section(random_c) uint8_t _bugBuffer1[FRAME_BUFFER_WIDTH_BYTES*1];
static volatile __section(random_c) uint8_t _frameBuffer2[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
static volatile __section(random_c) uint8_t _bugBuffer2[FRAME_BUFFER_WIDTH_BYTES*1];

static int16_t game_scroll;
static uint16_t game_paused;
static uint16_t game_gotoMenu;				 
static uint16_t game_singleStep;
static int16_t game_targetCameraY;
static int16_t game_shake;
static uint32_t game_lastScore;
static uint32_t game_lastLevelScore;

#ifdef DEBUG
static uint32_t game_lastVerticalBlankCount;
static int16_t game_turtle;
static uint16_t game_rasterLines[GAME_RASTERAVERAGE_LENGTH];
static uint16_t game_rasterLinesIndex;
static uint16_t game_maxRasterLine;
static uint16_t game_collectTotal;				 
static uint32_t game_total;
static uint32_t game_frame;
static uint32_t game_hwframe;
static uint16_t game_average;
static int16_t game_missedFrameCount;
static uint16_t game_lastAverage;
static uint16_t game_lastMaxRasterLine;
static int16_t game_lastEnemyCount;
static int16_t game_lastItemCount;
static int16_t game_lastMissedFrameCount;
static int16_t game_scoreBoardMode;
static int16_t game_debugRenderFrame;
#endif

static void (*game_tileRender)(uint16_t hscroll, uint16_t itemY);

 __section(data_c)  copper_t copper  = {
#ifdef PLAYER_HARDWARE_SPRITE
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
#endif
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
  game_onScreenBuffer = (frame_buffer_t)&_frameBuffer1;
  game_offScreenBuffer = (frame_buffer_t)&_frameBuffer2;
}


__EXTERNAL void
game_init(menu_command_t command)
{
  hw_waitVerticalBlank();
  palette_black();

  screen_setup((uint16_t*)&copper);
  screen_pokeCopperList(game_scoreBoardFrameBuffer, copper.bpl3);
  
  game_newGame(command);
}


static void
game_renderScore(uint16_t refresh)
{
  if (game_levelComplete) {
    if (game_levelScore >= GAME_LEVEL_BONUS_TRANSFER_RATE<<2) {
      game_score+=GAME_LEVEL_BONUS_TRANSFER_RATE;
      game_levelScore-=(GAME_LEVEL_BONUS_TRANSFER_RATE<<2);
    } else if (game_levelScore > 0){
      game_score += game_levelScore>>2;
      game_levelScore = 0;
    }
  } else if (!game_over) {
    game_levelScore--;
  }

#ifdef DEBUG
  if (game_scoreBoardMode == 0) {
#endif
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
      game_lastScore = game_score;
    }
#ifdef DEBUG
  }
#endif
}


static void
game_refreshScoreboard(void)
{
  game_lastScore = 1;  
  game_lastLevelScore = 0;

#ifdef DEBUG
  if (game_scoreBoardMode == 0) {
#endif

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
    game_renderScore(1);
    uint32_t i, x;
    for (i = 0, x = (SCREEN_WIDTH/2)-17; i < game_lives; i++, x+=10) {
      gfx_renderSprite(game_scoreBoardFrameBuffer, 208, 176, x, 5, 16, 8);
    }
    
    for (; i < 3; i++, x+= 10) {
      gfx_renderSprite(game_scoreBoardFrameBuffer, 208, 184, x, 5, 16, 8);
    }
#ifdef DEBUG
  }
#endif
}

void
game_overCallback(void)
{
  game_gotoMenu = 1;  
  hiscore_addScore(game_score);
}

void
game_finish(void)
{
  game_collisions = 0;
  game_over = 1;
  popup("GAME OVER!", game_overCallback);
}

void
game_loseLife(void)
{ 
  if (game_lives > 1) {
    game_lives--;
    game_refreshScoreboard();
  } else {
    game_finish();
  }
}


#ifdef DEBUG
static void
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
#endif


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

  game_cameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
  game_targetCameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
  game_scroll = 0;
  game_singleStep = 0;
  game_paused = 0;
  game_over = 0;
  game_gotoMenu = 0;
  game_screenScrollY = 0;
  game_shake = 0;
  game_requestCameraY(WORLD_HEIGHT-SCREEN_HEIGHT);
  game_levelScore = 9999<<2;
  game_levelComplete = 0;
  game_lastScore = 1;
  game_lastLevelScore = 0;

#ifdef DEBUG
  game_collectTotal = 1;
  game_turtle = 0;
  game_total = 0;
  game_frame  = 0;
  game_hwframe = 0;
  game_average = 0;
  game_maxRasterLine = 0;
  game_rasterLinesIndex = 0;
  game_lastMissedFrameCount = -1;
  game_missedFrameCount = 0;
  game_lastAverage = -1;
  game_lastMaxRasterLine = -1;
  game_lastEnemyCount = -1;
  game_lastItemCount = -1;
  game_scoreBoardMode = 0;
  game_collisions = 1;
  game_debugRenderFrame = 0;
#endif

  popup_off();

  game_switchFrameBuffers();

  sound_init();
  item_init(); // this must be initialised before tile
  enemy_init(); // this must be initialised before tile

  level_load(game_level);

  tile_init();
  tile_renderScreen();

  player_init(command);

  cloud_init();

  game_refreshScoreboard();

  hw_waitBlitter();

  game_render();

  game_switchFrameBuffers();

  game_render();

  hw_waitVerticalBlank();

  palette_fadeIn(level.fadeIn);

  hw_waitVerticalBlank();
  hw_verticalBlankCount = 0;
#ifdef DEBUG
  game_lastVerticalBlankCount = 0;
#endif
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
  game_shake = 4;
}


static void
game_moveCameraY(void)
{
  int16_t error = game_targetCameraY-game_cameraY;

#define SCROLL_ERROR_HIGHT 48
#define SCROLL_DELTA 1

  if (error > 0) {
    // falling
    if (error >= SCROLL_ERROR_HIGHT) {
      game_scroll-=SCROLL_DELTA;
    } else {
      if (game_scroll < -SCROLL_PIXELS) {
	game_scroll+=SCROLL_DELTA;
      } else if (game_scroll > -SCROLL_PIXELS) {
	game_scroll-=SCROLL_DELTA;
      };
    }
    if (game_scroll < -PHYSICS_TERMINAL_VELOCITY) {
      game_scroll = -PHYSICS_TERMINAL_VELOCITY;
    }
  } else if (error < 0) {
    // jumping
    game_scroll+=SCROLL_DELTA;
    if (game_scroll > SCROLL_PIXELS) {
      game_scroll = SCROLL_PIXELS;
    }
  } else {
    game_scroll  = 0;
  }

  if (abs(error) < abs(game_scroll)) {
    game_scroll =  -error;
  }

  if (game_scroll >= 0) {
    game_tileRender = tile_renderNextTile;
  } else {
    game_tileRender = tile_renderNextTileDown;
  }

  game_cameraY -= game_scroll;

  game_screenScrollY = -((game_cameraY-(WORLD_HEIGHT-SCREEN_HEIGHT)) % FRAME_BUFFER_HEIGHT);
}


static void
game_scrollBackground(void)
{
  int16_t screenScrollSave = game_screenScrollY;
  uint16_t cameraYSave = game_cameraY;
  game_moveCameraY();
  int16_t count = abs(game_scroll);
  int16_t tileY;
  uint16_t itemY;

  gfx_setupRenderTileOffScreen();

#if 0
  for (int s = 0, sy = screenScrollSave, cy = cameraYSave;  s < (count); s++) {
    if (game_scroll > 0) {
      sy++;
      cy--;
      tileY = (((sy-1) >> 4) << 4);
      itemY = cy+1;
    } else {
      sy--;
      cy++;
      tileY = (((sy+1) >> 4) << 4);
      itemY = cy-1;
    }
    (*game_tileRender)(tileY, itemY);
  }
#else
  if (game_scroll > 0) {
    for (int s = 0, sy = screenScrollSave, cy = cameraYSave;  s < (count); s++) {
      sy++;
      cy--;
      tileY = (((sy-1) >> 4) << 4);
      itemY = cy+1;
      (*game_tileRender)(tileY, itemY);
    }
  } else {
    for (int s = 0, sy = screenScrollSave, cy = cameraYSave;  s < (count); s++) {
      sy--;
      cy++;
      tileY = (((sy+1) >> 4) << 4);
      itemY = cy-1;
      (*game_tileRender)(tileY, itemY);
    }    
  }
#endif
}



#ifdef DEBUG
static void

debug_mode3(void)
{ 
  switch (game_debugRenderFrame) {
  case 0:
    if (game_average != game_lastAverage) {
      text_drawScoreBoard(text_intToAscii(game_average, 4), 0);
      game_lastAverage = game_average;
    }
    break;
  case 1:
    if (game_maxRasterLine != game_lastMaxRasterLine) {
      text_drawScoreBoard(text_intToAscii(game_maxRasterLine, 4), 5*8);
      game_lastMaxRasterLine = game_maxRasterLine;
    }
    break;
  case 2:
    {
      int16_t enemy_count = enemy_getCount();
      if (enemy_count != game_lastEnemyCount) {
	text_drawScoreBoard(text_intToAscii(enemy_count, 3), 10*8);
	game_lastEnemyCount = enemy_count;
      }
    }
    break;
  case 3:
    {
      int16_t item_count = item_getCount();
      if (item_count != game_lastItemCount) {
	text_drawScoreBoard(text_intToAscii(item_count, 2), 14*8);
	game_lastItemCount = item_count;
      }
    }
    break;
  default:
    break;
  } 
  game_debugRenderFrame++;
  if (game_debugRenderFrame > 3) {
    game_debugRenderFrame = 0;
  }
}

void
debug_mode1(void)
{ 
  switch (game_debugRenderFrame) {
  case 0:
    text_drawScoreBoard(itoa(game_total), 0);
    break;
  case 2:
    text_drawScoreBoard(itoa(game_frame), 9*8);
    break;
  case 3:
    text_drawScoreBoard(itoa(game_hwframe), 18*8);
    break;
  case 4:
    text_drawScoreBoard(text_intToAscii(game_missedFrameCount, 3),27*8);
    break;
  default:
    break;
  } 
  game_debugRenderFrame++;
  if (game_debugRenderFrame > 4) {
    game_debugRenderFrame = 0;
  }
}


static void
debug_showRasterLine(void)
{  
  if (hw_getRasterLine() < GAME_SCORE_RASTERLINE_CUTOFF || game_levelComplete) {
    switch (game_scoreBoardMode) {
    case 1:
      debug_mode1();
      break;
    case 2:
      text_drawScoreBoard(text_intToAscii(game_cameraY, 4), 0);
      text_drawScoreBoard(text_intToAscii(game_targetCameraY, 4), 5*8);
      if (game_scroll > 0) {
	text_drawScoreBoard(" ", 10*8);
	text_drawScoreBoard(text_intToAscii(game_scroll, 4), 11*8);
      } else {
	text_drawScoreBoard("-", 10*8);
	text_drawScoreBoard(text_intToAscii(-game_scroll, 4), 11*8);
      }
      break;
    case 3:
      debug_mode3();
      break;
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

  if (game_collectTotal) {
    game_total += line;
    game_frame++;
    game_hwframe = hw_verticalBlankCount;
  }

  return;  
}
#endif


static void
game_render(void)
{
  tile_renderInvalidTiles(game_offScreenBuffer);

  item_saveBackground(game_offScreenBuffer);

  enemy_saveBackground(game_offScreenBuffer);

  popup_saveBackground(game_offScreenBuffer);

#ifndef PLAYER_HARDWARE_SPRITE
  player_saveBackground(game_offScreenBuffer);
#endif

  if (level.clouds) {
    cloud_saveBackground(game_offScreenBuffer);
  }
  
  if (level.clouds) {
    cloud_render(game_offScreenBuffer);
  }

  item_render(game_offScreenBuffer);  
  enemy_render(game_offScreenBuffer);  
  popup_render(game_offScreenBuffer);

#ifndef PLAYER_HARDWARE_SPRITE
  player_render(game_offScreenBuffer);
#endif

}


void
game_requestCameraY(int16_t targetCameraY)
{
  game_targetCameraY = targetCameraY;

  if (game_targetCameraY < 0) {
    game_targetCameraY = 0;
  } else if (game_targetCameraY > WORLD_HEIGHT-SCREEN_HEIGHT) {
    game_targetCameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
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



static void
game_finishLevel(void)
{
  game_score += game_levelScore;
  game_playLevel(game_level + 1);
}


void
game_setLevelComplete(void)
{
  if (!game_levelComplete) {
    popup("LEVEL COMPLETE!", game_finishLevel);
    game_levelComplete = 1;
    game_collisions = 0;
  }
}


void
game_startPlayback(void)
{
  game_score = 0;
  game_lives = 3;

  palette_black();
  music_restart();
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
game_processKeyboard()
{
  switch (keyboard_key) {
#ifdef DEBUG
  case 'O':
    {
      game_finish();
      break;
      
    }
    break;
  case 'D':
    game_scoreBoardMode++;
    if (game_scoreBoardMode > 3) {
      game_scoreBoardMode = 0;
    }
    switch (game_scoreBoardMode) {
    case 0:
      game_collisions = 1;
      gfx_fillRect(game_scoreBoardFrameBuffer, 0, 0, FRAME_BUFFER_WIDTH, SCOREBOARD_HEIGHT, 0);
      game_refreshScoreboard();
      break;
    case 1:
    case 2:
    case 3:
      game_collisions = 0;
      game_refreshDebugScoreboard();
      break;
    }
    break;
  case 'X':
    game_collectTotal = !game_collectTotal;
    break;
    
#endif
#ifdef GAME_RECORDING
  case 'A':
    record_showAddress();
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
#endif
  case 'C':
    game_setLevelComplete();
    break;
  case 'T':
    game_singleStep = 1;
    break;
  case ' ':
    game_paused = !game_paused;
    break;
  case 'Z':
    music_next();
    break;
  case 'N':
    game_playLevel(game_level+1);
    break;
  case 'M':
    music_toggle();
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

  return 0;
}



__EXTERNAL void
game_loop()
{
  game_ctor();

  message_loading("Welcome to Climby Skies!");

  music_play(0);   

  hw_interruptsInit(); // Don't enable interrupts until music is set up

  music_toggle();

  hiscore_ctor();

  menu_command_t menuCommand;
 menu:
  game_level = 0;
  if ((menuCommand = menu_loop(game_over == 1 ? MENU_MODE_HISCORES : MENU_MODE_MENU)) == MENU_COMMAND_EXIT) {
#if TRACKLOADER==0
    goto done;
#endif
  }

  if (menuCommand == MENU_COMMAND_MENU) {
    goto menu;
  }

  game_init(menuCommand);

  for (;;) {    
    keyboard_read();
    hw_readJoystick();

    record_process();

    if (game_processKeyboard()) {
      goto menu;
    }
      
#ifdef DEBUG
    if (game_paused && game_singleStep != 1) {
      goto skip;
    }
    game_singleStep = 0;
#endif
    
    player_update();
    enemy_update();
    item_update();

    if (game_shake == 0) {
      if (level.clouds && game_scroll) {
	cloud_update(game_scroll >> 2);
      }
    } else if (game_shake > 0) {
      if (game_cameraY == WORLD_HEIGHT-SCREEN_HEIGHT) {
	game_requestCameraY(WORLD_HEIGHT-SCREEN_HEIGHT - 12);
	game_shake--;
      } else if (game_cameraY == WORLD_HEIGHT-SCREEN_HEIGHT-12){
	game_requestCameraY(WORLD_HEIGHT-SCREEN_HEIGHT);
	game_shake--;
      }
    } 

#ifdef DEBUG
    if (game_turtle > 1) {
      custom->color[16] = 0xf00;
      game_turtle--;
    } else if (game_turtle == 1) {
      custom->color[16] = 0x09e;
      game_turtle--;
    }
#endif

    if (hw_getRasterLine() < GAME_SCORE_RASTERLINE_CUTOFF || game_levelComplete) {
	game_renderScore(0);
    }
      
#ifdef DEBUG
    debug_showRasterLine();
#endif
    
    sound_schedule();

    if (game_gotoMenu) {
      goto menu;
    }

    hw_waitVerticalBlank();
#ifdef PLAYER_HARDWARE_SPRITE
    // this was before hw_waitVerticalBlank but caused glitches when things went too fast
    player_hSpriteRender();
#endif

    sound_vbl();

#ifdef PLAYER_HARDWARE_SPRITE
    player_updateCopper();
#endif
    
#ifdef DEBUG
    if (hw_verticalBlankCount-game_lastVerticalBlankCount > 1) {
      game_missedFrameCount++;
      game_turtle = 5;
    }
      
    game_lastVerticalBlankCount = hw_verticalBlankCount;
#endif

    game_switchFrameBuffers();

    if (game_cameraY != game_targetCameraY) {
      game_scrollBackground();
      if (game_cameraY == game_targetCameraY) {
	game_scroll = 0;
      }
    } 
     
    enemy_restoreBackground();
    item_restoreBackground();
    popup_restoreBackground();

#ifndef PLAYER_HARDWARE_SPRITE
    player_restoreBackground();
#endif

    if (level.clouds) {
      cloud_restoreBackground();
    }

    game_render();

#ifdef DEBUG
  skip:;
#endif
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
