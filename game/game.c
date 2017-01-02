#include "game.h"

volatile __chip uint8_t _frameBuffer[SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile uint8_t* frameBuffer;
static int hscroll = 0;
static int scroll = 1;


typedef struct {
  uint16_t bpl1[SCREEN_BIT_DEPTH*2*2];
  uint16_t wait1[2];
  uint16_t wait2[2];
  uint16_t bpl2[SCREEN_BIT_DEPTH*2*2];
  uint16_t end[2];
} copper_t;


static 
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
  .end = { 
    0xffff,0xfffe 
  }
};

void
game_init()
{
  palette_install();
  frameBuffer = (uint8_t*)&_frameBuffer;
  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_MASTER);
  gfx_fillRect(frameBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  screen_setup(frameBuffer, copper.bpl1);
  tile_renderScreen(frameBuffer);
}

// check for joystick button up
static uint16_t
joystickPressed()
{
  if (hw_joystickButton & 0x1) {
    for (;;) {
      hw_readJoystick();
      if (!(hw_joystickButton & 0x1)) {    
	break;
      }
    }
    return 1;
  }
  return 0;
}


static void
scrollBackground()
{
  hscroll+=scroll;
  
  hscroll = hscroll % FRAME_BUFFER_HEIGHT;
  
  uint16_t copperLine = RASTER_Y_START+hscroll;
  
  if (copperLine < 256) {
    copper.wait1[0] = (copperLine<<8)|1;
    copper.wait2[0] = (copperLine<<8)|1;
  } else if (copperLine >= 256) {
    copper.wait1[0] = 0xffdf;
    copper.wait2[0] = ((copperLine-256)<<8)|1;
  }

  screen_pokeCopperList(frameBuffer+((FRAME_BUFFER_HEIGHT-hscroll)*SCREEN_BIT_DEPTH*SCREEN_WIDTH_BYTES), copper.bpl1);
  screen_pokeCopperList(frameBuffer, copper.bpl2);
  
  if (hscroll % TILE_HEIGHT == 0) {	 
    if (tile_renderNextRow(frameBuffer,  hscroll)) {
      scroll = 0;
    }
  }
}

void
game_loop()
{
  int frame = 0;
  int done = 0;

  while (!done) {
    frame++;
    hw_readJoystick();
    hw_waitVerticalBlank();
    
    if (frame % 2 == 0) {
      scrollBackground();
    }

#if TRACKLOADER==0
    done = mouse_leftButtonPressed();
#endif
  }
}
