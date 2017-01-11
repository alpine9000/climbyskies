#include "game.h"

volatile __chip uint8_t _frameBuffer[FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile uint8_t* frameBuffer;
int cameraY = 0;
int scrollCount = 0;
static int scroll = SCROLL_PIXELS;

#ifdef SPRITE_BACKGROUND
static int lastLine = 0;
#endif


copper_t copper = {
#ifndef SPRITE_BACKGROUND
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
#else
  .wait = {0x0001, 0xff00},
  .sprites = {
      SPR0PTH, 0x0000,
      SPR0PTL, 0x0000,
      SPR1PTH, 0x0000,
      SPR1PTL, 0x0000,
      SPR2PTH, 0x0000,
      SPR2PTL, 0x0000,
      SPR3PTH, 0x0000,
      SPR3PTL, 0x0000,
      SPR4PTH, 0x0000,
      SPR4PTL, 0x0000,
      SPR5PTH, 0x0000,
      SPR5PTL, 0x0000,
      SPR6PTH, 0x0000,
      SPR7PTL, 0x0000,
      SPR7PTH, 0x0000,
      SPR7PTL, 0x0000
  },
#endif
  .end = {0xFFFF, 0xFFFE}
};

#ifdef SPRITE_BACKGROUND
static void 
initCopper(void)
{
  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    uint16_t copperLine = RASTER_Y_START+y;
    
    if (copperLine <= 255) {
      copper.lines[y].wait1[0] = copperLine<<8|1;
      copper.lines[y].wait1[1] = 0xff00;
      copper.lines[y].wait2[0] = copperLine<<8|1;
      copper.lines[y].wait2[1] = 0xff00;
    } else if (copperLine == 256) {
      copper.lines[y].wait1[0] = 0xffdf;
      copper.lines[y].wait1[1] = 0xfffe;
      copper.lines[y].wait2[0] = 1;
      copper.lines[y].wait2[1] = 0xff00;
    } else if (copperLine > 256) {
      copper.lines[y].wait1[0] = (copperLine-256)<<8|1;
      copper.lines[y].wait1[1] = 0xff00;
      copper.lines[y].wait2[0] = (copperLine-256)<<8|1;
      copper.lines[y].wait2[1] = 0xff00;
    }

    for (int b = 0; b < SCREEN_BIT_DEPTH; b++) {
      copper.lines[y].bpl[b*4] = COP2LCH;// TODO: jump
      copper.lines[y].bpl[(b*4)+2] =  COP2LCL;
      copper.lines[y].bpl[(b*4)+1] = 0xff00;
      copper.lines[y].bpl[(b*4)+3] = 0xff00;
    }

    for (int c = 0; c < NUM_SPRITE_COLUMNS; c++) {
      copper.lines[y].sprite[c].wait[0] = (0x40+((c*NUM_SPRITES*16)/4))|1;
      copper.lines[y].sprite[c].wait[1] = 0x00fe;
      for (int s = 0; s < NUM_SPRITES; s++) {
	copper.lines[y].sprite[c].pos[s*2] = SPR0POS+(8*s);
	copper.lines[y].sprite[c].pos[(s*2)+1] = ((0x91+(s*16))+(c*NUM_SPRITES*16)) >> 1;
      }
    }
  }
}
#endif

#ifdef SPRITE_BACKGROUND
static void 
clearCopper(void)
{

  for (int b = 0; b < SCREEN_BIT_DEPTH; b++) {
    copper.lines[lastLine].bpl[(b*4)+0] = BPL6PTL;
    copper.lines[lastLine].bpl[(b*4)+1] = 0x0;
    copper.lines[lastLine].bpl[(b*4)+2] = BPL6PTL;
    copper.lines[lastLine].bpl[(b*4)+3] = 0x0;
  }
}

static void 
pokeCopper(int16_t line, frame_buffer_t ptr)
{
  uint32_t bitplanesPtr = (uint32_t)ptr;
  
  for (int b = 0; b < SCREEN_BIT_DEPTH; b++) {
    copper.lines[line].bpl[(b*4)+0] = BPL1PTL + (b*4);
    copper.lines[line].bpl[(b*4)+2] = BPL1PTH + (b*4);
    copper.lines[line].bpl[(b*4)+1] = bitplanesPtr;
    copper.lines[line].bpl[(b*4)+3] = (bitplanesPtr >> 16);
    bitplanesPtr = bitplanesPtr + (FRAME_BUFFER_WIDTH_BYTES);
  }
}
#endif

void
game_init()
{
  palette_install();
  frameBuffer = (uint8_t*)&_frameBuffer;
#ifdef SPRITE_BACKGROUND
  initCopper();
  pokeCopper(0, frameBuffer);
#else
  screen_pokeCopperList(frameBuffer, copper.bpl1);
#endif
  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_MASTER);
  gfx_fillRect(frameBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  screen_setup(frameBuffer, (uint16_t*)&copper);
  tile_renderScreen(frameBuffer);
  actor_render(frameBuffer);
#ifdef SPRITE_BACKGROUND
  sprite_init();
#endif
}

#if 0
// check for joystick button up
static uint16_t
joystickPressed()
{
  if (JOYSTICK_BUTTON_DOWN) {
    for (;;) {
      hw_readJoystick();
      if (!JOYSTICK_BUTTON_DOWN) {    
	break;
      }
    }
    return 1;
  }
  return 0;
}
#endif


void
scrollBackground()
{
  cameraY += scroll;
  int hscroll = cameraY % FRAME_BUFFER_HEIGHT;

#ifndef SPRITE_BACKGROUND
  uint16_t copperLine = RASTER_Y_START+hscroll;
  
  if (copperLine < 256) {
    copper.wait1[0] = (copperLine<<8)|1;
    copper.wait2[0] = (copperLine<<8)|1;
  } else if (copperLine >= 256) {
    copper.wait1[0] = 0xffdf;
    copper.wait2[0] = ((copperLine-256)<<8)|1;
  }

  screen_pokeCopperList(frameBuffer+((FRAME_BUFFER_HEIGHT-hscroll)*SCREEN_BIT_DEPTH*FRAME_BUFFER_WIDTH_BYTES), copper.bpl1);
  screen_pokeCopperList(frameBuffer, copper.bpl2);
  
#else
    
  clearCopper();

  if (hscroll != 0) {
    pokeCopper(0, frameBuffer+((FRAME_BUFFER_HEIGHT-hscroll)*SCREEN_BIT_DEPTH*FRAME_BUFFER_WIDTH_BYTES));
  }

  if (hscroll < SCREEN_HEIGHT) {
    pokeCopper(hscroll, frameBuffer);
    lastLine = hscroll;
  }
#endif

  static int tileY = 0;
  
  int tileIndex = hscroll % TILE_HEIGHT;

  for (int s = 0;  s < scroll && tileIndex+s < SCREEN_WIDTH/TILE_HEIGHT; s++) {
    if (tile_renderNextTile(frameBuffer, tileY)) {
      if (scroll != 0) {
	scroll = 0;
	music_play(1);
      }
    }
  }

  if (tileIndex == 0) {	 
     tileY = hscroll;
  }
}

void
game_loop()
{
  int frame = 0;
  int done = 0;
  int joystickDown = 1;

  music_play(0);
  // Don't enable interrupts until music is set up
   hw_interruptsInit();

  while (!done) {
    frame++;
    hw_readJoystick();

    if (scrollCount == 0 && !joystickDown && JOYSTICK_BUTTON_DOWN) {    
      scrollCount = 1+((6*16)/SCROLL_PIXELS);
      joystickDown = 1;
    }
    joystickDown = JOYSTICK_BUTTON_DOWN;


    hw_waitVerticalBlank();

     custom->color[0] = 0xf00;    
    

#ifdef SPRITE_BACKGROUND
    if (scrollCount > 0 && frame % 1 == 0 && scroll) {
      sprite_scroll(-2);
    }
#endif

    // 812
    // 7 3
    // 654 


    if (hw_joystickPos == 1) {
      actor_jump();
    }
    if (hw_joystickPos == 7) {
      actor_left();
    }
    if (hw_joystickPos == 3) {
      actor_right();
    }

    if (scrollCount > 1) {
      scrollBackground();
      scrollCount--;
    } else if (scrollCount > 0) {
      scrollCount--;
    } else {
      actor_render(frameBuffer);    
    }



    custom->color[0] = 0xff0;        
       hw_waitRaster(100);
    custom->color[0] = 0xfff;    
    //    actor_render(frameBuffer);    
    hw_waitBlitter();
    custom->color[0] = 0x00f;    
    
#if TRACKLOADER==0
    done = mouse_leftButtonPressed();
#endif
  }
}
