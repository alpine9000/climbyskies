#include "game.h"

volatile __chip uint8_t _frameBuffer1[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile __chip uint8_t _saveBuffer1[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile __chip uint8_t _frameBuffer2[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile __chip uint8_t _saveBuffer2[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
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
  //  .color1 = {
  //    COLOR01, 0x00f,
  //  },
  //  .color2 = {
  //    COLOR01, 0xf00,
  //  },

  .end = {0xFFFF, 0xFFFE}
};


void
game_init()
{
  palette_install();
  onScreenBuffer = (uint8_t*)&_frameBuffer1;
  offScreenBuffer = (uint8_t*)&_frameBuffer2;
  saveBuffer = (uint8_t*)&_saveBuffer1;
  saveBuffer1 = (uint8_t*)&_saveBuffer1;
  saveBuffer2 = (uint8_t*)&_saveBuffer2;
  screen_pokeCopperList(offScreenBuffer, copper.bpl1);
  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_MASTER);
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
  } else if (copperLine >= 256) {
    copper.wait1[0] = 0xffdf;
    copper.wait2[0] = ((copperLine-256)<<8)|1;
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

  uint16_t copperLine = RASTER_Y_START+screenScrollY;
  
  if (copperLine < 256) {
    copper.wait1[0] = (copperLine<<8)|1;
    copper.wait2[0] = (copperLine<<8)|1;
  } else if (copperLine >= 256) {
    copper.wait1[0] = 0xffdf;
    copper.wait2[0] = ((copperLine-256)<<8)|1;
  }

  screen_pokeCopperList(onScreenBuffer+(int)dyOffsetsLUT[FRAME_BUFFER_HEIGHT-screenScrollY], copper.bpl1);
  //  screen_pokeCopperList(onScreenBuffer+((FRAME_BUFFER_HEIGHT-screenScrollY)*SCREEN_BIT_DEPTH*FRAME_BUFFER_WIDTH_BYTES), copper.bpl1);
  screen_pokeCopperList(onScreenBuffer, copper.bpl2);
  
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
      scrollCount = 2;//1+((6*16)/SCROLL_PIXELS);
      joystickDown = 1;
    }
    joystickDown = JOYSTICK_BUTTON_DOWN;

    player_update();
    cloud_update();


    hw_waitVerticalBlank();
    //    custom->color[2] = 0xf00;
    switchFrameBuffers();

    if (scrollCount > 1) {
      scrollBackground();
      scrollCount--;
    } else if (scrollCount > 0) {
      scrollCount--;
    }

    player_restoreBackground();
    cloud_restoreBackground();
    
    player_saveBackground(offScreenBuffer);
    cloud_saveBackground(offScreenBuffer);

    //    custom->color[2] = 0x0f0;
    cloud_render(offScreenBuffer);
    //    custom->color[2] = 0x0fF;
    player_render(offScreenBuffer);
    saveBuffer = saveBuffer == saveBuffer1 ? saveBuffer2 : saveBuffer1;    

    hw_waitBlitter();
    //    custom->color[2] = 0x000;

#if TRACKLOADER==0
    done = mouse_leftButtonPressed();
#endif
  }
}
