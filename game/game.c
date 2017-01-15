#include "game.h"

//#define SHOW_SPEED 1

#ifdef SHOW_SPEED
#define SPEED_COLOR(x) custom->color[1] = x;
#else
#define SPEED_COLOR(x) 
#endif


volatile __chip uint8_t _frameBuffer1[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile __chip uint8_t _saveBuffer1[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile __chip uint8_t _frameBuffer2[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile __chip uint8_t _saveBuffer2[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile __chip uint8_t _scoreBoardBuffer[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(SCOREBOARD_HEIGHT)];

frame_buffer_t scoreBoardFrameBuffer;
frame_buffer_t offScreenBuffer;
frame_buffer_t onScreenBuffer;
frame_buffer_t saveBuffer;
frame_buffer_t saveBuffer1;
frame_buffer_t saveBuffer2;
int cameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
int screenScrollY = 0;
int scrollCount = 0;
uint32_t frameCount = 0;
static int scroll = SCROLL_PIXELS;

copper_t copper = {
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


static void
switchFrameBuffers(void);

void
game_init()
{
  hw_waitVerticalBlank();
  palette_black();

  onScreenBuffer = (frame_buffer_t)&_frameBuffer1;
  scoreBoardFrameBuffer = (frame_buffer_t)&_scoreBoardBuffer;
  offScreenBuffer = (frame_buffer_t)&_frameBuffer2;
  saveBuffer = (frame_buffer_t)&_saveBuffer1;
  saveBuffer1 = (frame_buffer_t)&_saveBuffer1;
  saveBuffer2 = (frame_buffer_t)&_saveBuffer2;
  screen_setup(onScreenBuffer, (uint16_t*)&copper);
  screen_pokeCopperList(scoreBoardFrameBuffer, copper.bpl3);

  switchFrameBuffers();

  tile_renderScreen();
  player_init(offScreenBuffer);
  cloud_init(offScreenBuffer);

  gfx_fillRect(scoreBoardFrameBuffer, 0, 0, FRAME_BUFFER_WIDTH, SCOREBOARD_HEIGHT, 0);

  hw_waitBlitter();
  hw_waitVerticalBlank();

  music_play(0);
   // Don't enable interrupts until music is set up
  hw_interruptsInit();

  palette_fadeIn();
}


static void
switchFrameBuffers(void)
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


static void
scrollBackground()
{
  static int tileY = 0;

  cameraY -= scroll;
  screenScrollY = -((cameraY-(WORLD_HEIGHT-SCREEN_HEIGHT)) % FRAME_BUFFER_HEIGHT);
  
  int tileIndex = screenScrollY % TILE_HEIGHT;

  for (int s = 0;  s < scroll && tileIndex+s < SCREEN_WIDTH/TILE_HEIGHT; s++) {
    if (tile_renderNextTile(tileY)) {
      if (scroll != 0) {
	scroll = 0;
	//  music_play(1);
      }
    }
  }

  if (tileIndex == 0) {	 
     tileY = screenScrollY;
  }
}


static void
debug_showRasterLine(void)
{
#define AVERAGE_LENGTH 16
  static int rasterLines[AVERAGE_LENGTH];
  static int rasterLinesIndex = 0;
  static int maxRasterLine = 0;
  
  int line = hw_getRasterLine() - RASTER_Y_START;
  rasterLines[rasterLinesIndex++] = line;
  if (line > maxRasterLine) {
    maxRasterLine = line;
  }
  if (rasterLinesIndex >= AVERAGE_LENGTH) {
    rasterLinesIndex = 0;
  }
  int average = 0;
  for (int i = 0; i < AVERAGE_LENGTH; i++) {
    average += rasterLines[i];
  }
  average = average >> 4;
  

  text_drawText8(scoreBoardFrameBuffer, text_intToAscii(average, 4), 0, 4);  
  text_drawText8(scoreBoardFrameBuffer, text_intToAscii(maxRasterLine, 4), 5*8, 4);
}

void
game_loop()
{
  int done = 0;
  int joystickDown = 1;

  while (!done) {
    frameCount++;
    hw_readJoystick();

    if (scrollCount == 0 && !joystickDown && JOYSTICK_BUTTON_DOWN) {    
      scrollCount = 1;//1+((6*16)/SCROLL_PIXELS);
      joystickDown = 1;
    }
    joystickDown = JOYSTICK_BUTTON_DOWN;

    player_update();
    cloud_update();


    hw_waitVerticalBlank();
    SPEED_COLOR(0xf00);
    switchFrameBuffers();

    if (scrollCount >= 1) {
      scrollBackground();
      scrollCount--;
    } else if (scrollCount > 0) {
      //scrollCount--;
    }

    //    text_restore();
    player_restoreBackground();
    cloud_restoreBackground();
    
    player_saveBackground(offScreenBuffer);
    cloud_saveBackground(offScreenBuffer);

    SPEED_COLOR(0x0f0);
    cloud_render(offScreenBuffer);
    SPEED_COLOR(0x0ff);
    player_render(offScreenBuffer);
    saveBuffer = saveBuffer == saveBuffer1 ? saveBuffer2 : saveBuffer1;    

    hw_waitBlitter();
    SPEED_COLOR(0xfff);
    debug_showRasterLine();
    SPEED_COLOR(0x000);

#if TRACKLOADER==0
    done = mouse_leftButtonPressed();
#endif
  }
}
