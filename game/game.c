#include "game.h"

volatile __chip uint8 _frameBuffer[SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH*SCREEN_HEIGHT];
volatile uint8* frameBuffer;
volatile uint8* spriteFrameBuffer;

static __chip uint16 copper[] = {
  //;;;  bitplane pointers must be first else poking addresses will be incorrect
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
  0xffff, 0xfffe
};


void
a1942_init()
{
  
  palette_install();
  frameBuffer = (uint8*)&_frameBuffer;
  spriteFrameBuffer = (uint8*)&spriteBitplanes;
  frameBuffer = (uint8*)&spriteBitplanes;
  screen_setup(frameBuffer, copper);
  gfx_fillRect(frameBuffer, 32, 32, 100, 100, 3);
}


void
a1942_loop()
{
  int done = 0;
  while (!done) {
    hw_waitVerticalBlank();

    #if TRACKLOADER==0
    done = mouse_leftButtonPressed();
    #endif

    hw_readJoystick();
  }
}
