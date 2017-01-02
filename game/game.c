#include "game.h"

volatile __chip uint8_t _frameBuffer[SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH*(SCREEN_HEIGHT+32)];
volatile uint8_t* frameBuffer;


typedef struct {
  uint16_t wait1[2];
  uint16_t bpl1[SCREEN_BIT_DEPTH*2*2];
  uint16_t wait2[2];
  uint16_t wait3[2];
  uint16_t bpl2[SCREEN_BIT_DEPTH*2*2];
  uint16_t wait4[2];
} copper_t;


//static 
copper_t copper = {
  .wait1  = { 0x1d<<8|1,0xfffe },
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
  .wait2 = { ((0x1d+1)<<8)|1,0xfffe },
  .wait3 = { ((0x1d+1)<<8)|1,0xfffe },
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
  .wait4 = { 0xffff,0xfffe }
};

void
game_init()
{
  
  palette_install();
  frameBuffer = (uint8_t*)&_frameBuffer;
  screen_setup(frameBuffer, copper.bpl1);
  gfx_fillRect(frameBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 2);
#if 0
  gfx_fillRect(frameBuffer, 0, 0, 64, 64, 3);
  gfx_renderSprite(frameBuffer, 16, 16, 64, 64, 16, 16);
  gfx_renderTile(frameBuffer, 16, 16, 64+16, 64+16);
#else
  tile_renderScreen(frameBuffer, 0);
#endif
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


/*
   0 |      |
   1 +------+
   2 +------+
   3 |      |
   4 |      |
   5 |      |
   6 |      |
   7 |      |


   hscroll = 2%16

   line1 = (H-2)*
   line2 = (2+8-1)%8 = 1

*/

void
game_loop()
{
  int hscroll = 2;
  int done = 0;

  while (!done) {
    hw_waitVerticalBlank();
    hw_readJoystick();

    //    if (joystickPressed()) {
    if (hw_joystickButton & 0x1) {
      hscroll++;
    }

    hscroll = hscroll % SCREEN_HEIGHT;

    copper.wait1[0] = 0x1d<<8|1;
    copper.wait2[0] = ((0x1d+hscroll)<<8)|1;
    copper.wait3[0] = ((0x1d+hscroll)<<8)|1;
    screen_pokeCopperList(frameBuffer+((SCREEN_HEIGHT-hscroll)*SCREEN_BIT_DEPTH*SCREEN_WIDTH_BYTES), copper.bpl1);
    screen_pokeCopperList(frameBuffer, copper.bpl2);
    #if TRACKLOADER==0
    done = mouse_leftButtonPressed();
    #endif


  }
}
