#include "game.h"

volatile __chip uint8_t _frameBuffer[SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH*(FRAME_BUFFER_HEIGHT)];
volatile uint8_t* frameBuffer;
static int hscroll = 0;
static int scroll = 1;

#define NUM_SPRITE_COLUMNS 3
#define NUM_SPRITES 4

typedef struct {
  uint16_t wait[2];
  uint16_t pos[NUM_SPRITES*2];
} sprite_pos_t;

typedef struct {
  uint16_t wait1[2];
  uint16_t wait2[2];
  uint16_t bpl[SCREEN_BIT_DEPTH*2*2];
  sprite_pos_t sprite[NUM_SPRITE_COLUMNS];
} copper_video_line_t;

typedef struct {
  uint16_t sprites[NUM_SPRITES*4];
  copper_video_line_t lines[SCREEN_HEIGHT];
  uint16_t end[2];
} copper_t;


copper_t copper = {
  .sprites = {
      SPR0PTH, 0x0000,
      SPR0PTL, 0x0000,
      SPR1PTH, 0x0000,
      SPR1PTL, 0x0000,
      SPR2PTH, 0x0000,
      SPR2PTL, 0x0000,
      SPR3PTH, 0x0000,
      SPR3PTL, 0x0000,
  },
  .end = {0xFFFF, 0xFFFE}
};

void setupCopper(frame_buffer_t fb)
{
  uint32_t bitplanesPtr = (uint32_t)frameBuffer;

  for (int y = 0; y < SCREEN_HEIGHT; y++) {
    uint16_t copperLine = RASTER_Y_START+y;
    
    if (copperLine <= 255) {
      copper.lines[y].wait1[0] = copperLine<<8|1;
      copper.lines[y].wait1[1] = 0xff00;
      copper.lines[y].wait2[0] = copperLine<<8|1;
      copper.lines[y].wait2[1] = 0xff00;
    } else if (copperLine == 256) {
      copper.lines[y].wait1[0] = 0xffdf
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
	copper.lines[y].bpl[b*4] = BPL1PTL + (b*4);
	copper.lines[y].bpl[(b*4)+2] = BPL1PTH + (b*4);
	copper.lines[y].bpl[(b*4)+1] = bitplanesPtr;
	copper.lines[y].bpl[(b*4)+3] = (bitplanesPtr >> 16);
	bitplanesPtr = bitplanesPtr + (SCREEN_WIDTH_BYTES);
      }
   

#if 1
    for (int c = 0; c < NUM_SPRITE_COLUMNS; c++) {
      copper.lines[y].sprite[c].wait[0] = (0x30+((c*NUM_SPRITES*16)/4))|1;
      copper.lines[y].sprite[c].wait[1] = 0x00fe;
      for (int s = 0; s < NUM_SPRITES; s++) {
	copper.lines[y].sprite[c].pos[s*2] = SPR0POS+(8*s);
	copper.lines[y].sprite[c].pos[(s*2)+1] = ((0x91+(s*16))+(c*NUM_SPRITES*16)) >> 1;
      }
    }
#endif
  }
}

/*
typedef struct {
  uint16_t bpl1[SCREEN_BIT_DEPTH*2*2];
  uint16_t sprite0[4];
  uint16_t sprite1[4];
  uint16_t wait1[2];
  uint16_t wait2[2];
  uint16_t bpl2[SCREEN_BIT_DEPTH*2*2];
  uint16_t end[2];
} copper_t;

//static 
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
  .sprite0 = {
    SPR0PTH, 0x0000,
    SPR0PTL, 0x0000,
  },
  .sprite1 = {
    SPR1PTH, 0x0000,
    SPR1PTL, 0x0000,
  },
  .wait1 = { 
    0xffff,0xfffe 
  },
  .sprite1 = {
    SPR1PTH, 0x0000,
    SPR1PTL, 0x0000,
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
*/

void
game_init()
{
  palette_install();
  frameBuffer = (uint8_t*)&_frameBuffer;
  setupCopper(frameBuffer);
  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_MASTER);
  gfx_fillRect(frameBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  //  screen_setup(frameBuffer, copper.bpl1);

  screen_setup(frameBuffer, &copper);
  tile_renderScreen(frameBuffer);


  copper.sprites[1] = (uint32_t)&spriteBackground0 >> 16;
  copper.sprites[3] = (uint32_t)&spriteBackground0;
  copper.sprites[5] = (uint32_t)&spriteBackground1 >> 16;
  copper.sprites[7] = (uint32_t)&spriteBackground1;
  copper.sprites[9] = (uint32_t)&spriteBackground2 >> 16;
  copper.sprites[0xb] = (uint32_t)&spriteBackground2;
  copper.sprites[0xd] = (uint32_t)&spriteBackground3 >> 16;
  copper.sprites[0xf] = (uint32_t)&spriteBackground3;


  //  volatile uint16_t* sp = &spriteBackground0;
  //sp[0] = (0x1d << 8) | (0x81)>>1;
  //  sp[1] = 0xff02;
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
  /*
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
    }*/
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
