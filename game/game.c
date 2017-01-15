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
#if SCOREBOARD_HEIGHT > 0
volatile __chip uint8_t _scoreBoardBuffer[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(SCOREBOARD_HEIGHT)];
#endif
#if SCOREBOARD_HEIGHT > 0
frame_buffer_t scoreBoardFrameBuffer;
#endif
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
    0xffff,0xfffe 
  },
  .wait2 = { 
    0xffff,0xfffe
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
#if SCOREBOARD_HEIGHT > 0
    (RASTER_Y_START)<<8|1, 0xFFFE,
    (RASTER_Y_START)<<8|1, 0xFFFE,
#else
    0xffff, 0xfffe,
    0xffff, 0xfffe,
#endif
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
  /*  .color2 = {
    COLOR00, 0x00F,
    COLOR01, 0x00F,
    },*/
  .end = {0xFFFF, 0xFFFE}
};


void
game_init()
{
  palette_install();
  onScreenBuffer = (uint8_t*)&_frameBuffer1;
#if SCOREBOARD_HEIGHT > 0
  scoreBoardFrameBuffer = (frame_buffer_t)&_scoreBoardBuffer;
#endif

  offScreenBuffer = (uint8_t*)&_frameBuffer2;
  saveBuffer = (uint8_t*)&_saveBuffer1;
  saveBuffer1 = (uint8_t*)&_saveBuffer1;
  saveBuffer2 = (uint8_t*)&_saveBuffer2;
  screen_pokeCopperList(offScreenBuffer, copper.bpl1);
  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_MASTER);
#if SCOREBOARD_HEIGHT > 0
  gfx_fillRect(scoreBoardFrameBuffer, 0, 0, FRAME_BUFFER_WIDTH, SCOREBOARD_HEIGHT, 0);
  screen_pokeCopperList(scoreBoardFrameBuffer, copper.bpl3);
#endif

  screen_setup(onScreenBuffer, (uint16_t*)&copper);
  tile_renderScreen();
  player_init(offScreenBuffer);
  cloud_init(offScreenBuffer);
}

static
void
switchFrameBuffers()
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
      //copper.wait2[0] = 0xffff;
    } else {
      copper.wait2[0] = ((copperLine-256)<<8)|1;
    }
    copper.wait3[0] = (RASTER_Y_START)<<8|1;
  }


  screen_pokeCopperList(offScreenBuffer+(int)dyOffsetsLUT[FRAME_BUFFER_HEIGHT-screenScrollY], copper.bpl1);
  //  screen_pokeCopperList(offScreenBuffer+((FRAME_BUFFER_HEIGHT-screenScrollY)*SCREEN_BIT_DEPTH*FRAME_BUFFER_WIDTH_BYTES), copper.bpl1);
  screen_pokeCopperList(offScreenBuffer, copper.bpl2);


  frame_buffer_t save = onScreenBuffer;
  onScreenBuffer = offScreenBuffer;
  offScreenBuffer = save;

}

static
void
scrollBackground()
{
  cameraY -= scroll;
  screenScrollY = -((cameraY-(WORLD_HEIGHT-SCREEN_HEIGHT)) % FRAME_BUFFER_HEIGHT);

  /*
  uint16_t copperLine = RASTER_Y_START+screenScrollY;

if (copperLine < 256) {
    copper.wait1[0] = (copperLine<<8)|1;
    copper.wait2[0] = (copperLine<<8)|1;
    copper.wait3[0] = 0xffdf;
  } else if (copperLine >= 256) {
    copper.wait1[0] = 0xffdf;
    if (screenScrollY > 256) {
      copper.wait2[0] = (RASTER_Y_START)<<8|1;
    } else {
      copper.wait2[0] = ((copperLine-256)<<8)|1;
    }
    copper.wait3[0] = (RASTER_Y_START)<<8|1;
    }

  screen_pokeCopperList(onScreenBuffer+(int)dyOffsetsLUT[FRAME_BUFFER_HEIGHT-screenScrollY], copper.bpl1);
  //  screen_pokeCopperList(onScreenBuffer+((FRAME_BUFFER_HEIGHT-screenScrollY)*SCREEN_BIT_DEPTH*FRAME_BUFFER_WIDTH_BYTES), copper.bpl1);
  screen_pokeCopperList(onScreenBuffer, copper.bpl2);
  */
  static int tileY = 0;
  
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
showRasterLine(void)
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

  music_play(0);
  // Don't enable interrupts until music is set up
   hw_interruptsInit();

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
    showRasterLine();
    SPEED_COLOR(0x000);

#if TRACKLOADER==0
    done = mouse_leftButtonPressed();
#endif
  }
}
