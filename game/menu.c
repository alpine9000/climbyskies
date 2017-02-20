#include "game.h"
// http://www.picturetopeople.org/text_generator/others/3d/3d-perspective-text-effect-creator.html

extern void palette_menuInstall(void);
extern frame_buffer_t menuFrameBuffer;

#define MENU_NUM_ITEMS             4
#define MENU_TOP_COLOR             0x7ef
#define MENU_BOTTOM_COLOR          0x5cd
#define MENU_TOP_COLOR_SELECTED    0xbe0
#define MENU_BOTTOM_COLOR_SELECTED 0x9d4

#define MENU_COPPER_WAIT_TOP(x) { 0x9fd1 + (0x800*(x*2)), 0xfffe}
#define MENU_COPPER_WAIT_BOTTOM(x) { 0x9fd1 + 0x400 + (0x800*(x*2)), 0xfffe}

typedef struct {
  uint16_t wait1[2];
  uint16_t color1[2];
  uint16_t wait2[2];
  uint16_t color2[2];
} menu_line_copper_t;

typedef struct {
  uint16_t bpl1[SCREEN_BIT_DEPTH*2*2];
  menu_line_copper_t lines[MENU_NUM_ITEMS];
  uint16_t end[2];
} menu_copper_t;

static  __section(data_c)  menu_copper_t menu_copper  = {
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

  .lines = {
    [0] = {
      .wait1 = MENU_COPPER_WAIT_TOP(0),
      .color1 = { COLOR07, MENU_TOP_COLOR_SELECTED}, 
      .wait2 =  MENU_COPPER_WAIT_BOTTOM(0),
      .color2 = { COLOR07, MENU_BOTTOM_COLOR_SELECTED}, 
    },
    [1] = {
      .wait1 = MENU_COPPER_WAIT_TOP(1),
      .color1 = { COLOR07, MENU_TOP_COLOR}, 
      .wait2 = MENU_COPPER_WAIT_BOTTOM(1),
      .color2 = { COLOR07, MENU_BOTTOM_COLOR}, 
    },
    [2] = {
      .wait1 = MENU_COPPER_WAIT_TOP(2),
      .color1 = { COLOR07, MENU_TOP_COLOR}, 
      .wait2 = MENU_COPPER_WAIT_BOTTOM(2),
      .color2 = { COLOR07, MENU_BOTTOM_COLOR}, 
    },
    [3] = {
      .wait1 = MENU_COPPER_WAIT_TOP(3),
      .color1 = { COLOR07, MENU_TOP_COLOR}, 
      .wait2 = MENU_COPPER_WAIT_BOTTOM(3),
      .color2 = { COLOR07, MENU_BOTTOM_COLOR}, 
    }
  },

  .end = {0xFFFF, 0xFFFE}

};

static void 
menu_pokeCopperList(frame_buffer_t frameBuffer)
{
  uint16_t volatile* copperPtr = (uint16_t*)&menu_copper;
  /* poke bitplane pointers into copper list */
  uint32_t bitplanesPtr = (uint32_t)frameBuffer;

  for (int16_t i = 0; i < SCREEN_BIT_DEPTH; i++) {
    copperPtr[1] = (uint16_t)bitplanesPtr;
    copperPtr[3] = (uint16_t)(((uint32_t)bitplanesPtr)>>16);
    bitplanesPtr = bitplanesPtr + (SCREEN_WIDTH_BYTES);
    copperPtr = copperPtr + 4;
  }
}


static int16_t
menu_processKeyboard(void)
{
  switch (keyboard_getKey() ) {
#if TRACKLOADER==0
  case 'Q':
    return 1;
    break;
#endif
  }

  return 0;
}

char* menu_items[MENU_NUM_ITEMS+1] = {
  "PLAY NOW!",
  "MUSIC - ON",
  "HI SCORES",
  "CREDITS",
  0
};


static int
_strlen(char* s) 
{
  int count = 0;
  while (*s++ != 0) {
    count++;
  }

  return count;
}


static void
menu_render(frame_buffer_t fb)
{
  fb += 2*SCREEN_WIDTH_BYTES;
  int y = 130;

  for (int i = 0; menu_items[i] != 0; i++) {
    text_drawMaskedText8Blitter(fb, menu_items[i], (SCREEN_WIDTH/2)-(_strlen(menu_items[i])<<2)+1, y+1);
    text_drawMaskedText8Blitter(fb, menu_items[i], (SCREEN_WIDTH/2)-(_strlen(menu_items[i])<<2), y);
    y+= 16;
  }
  
  y = 130;

  fb -= SCREEN_WIDTH_BYTES;
  for (int i = 0; menu_items[i] != 0; i++) {
    text_drawMaskedText8Blitter(fb, menu_items[i], (SCREEN_WIDTH/2)-(_strlen(menu_items[i])<<2), y);
    y+= 16;
  }
}


int16_t
menu_loop(void)
{
  volatile uint16_t scratch;

  hw_waitVerticalBlank();
  custom->dmacon = DMAF_RASTER|DMAF_SPRITE;

  palette_black();

  disk_loadData((void*)game_onScreenBuffer, (void*)menuFrameBuffer, SCREEN_WIDTH_BYTES*SCREEN_HEIGHT*SCREEN_BIT_DEPTH);

  uint16_t volatile* copperPtr = (uint16_t*)&menu_copper;

  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR);

  /* set up playfield */
  
  custom->diwstrt = (RASTER_Y_START<<8)|RASTER_X_START;
  custom->diwstop = ((RASTER_Y_STOP-256)<<8)|(RASTER_X_STOP-256);

  custom->ddfstrt = (RASTER_X_START/2-SCREEN_RES);
  custom->ddfstop = (RASTER_X_START/2-SCREEN_RES)+(8*((SCREEN_WIDTH/16)-1));
  custom->bplcon0 = (SCREEN_BIT_DEPTH<<12)|0x200;
  custom->bpl1mod = (SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH)-SCREEN_WIDTH_BYTES;
  custom->bpl2mod = (SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH)-SCREEN_WIDTH_BYTES;

  /* install copper list, then enable dma and selected interrupts */
  custom->cop1lc = (uint32_t)copperPtr;
  scratch = custom->copjmp1;

  USE(scratch);

  menu_pokeCopperList(game_onScreenBuffer);

  hw_waitVerticalBlank();

  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_COPPER|DMAF_RASTER|DMAF_MASTER);
  palette_menuFadeIn();

  hw_waitVerticalBlank();
  menu_render(game_onScreenBuffer);
  int exit = 0;

  while (!exit) {
    hw_readJoystick();
    if (JOYSTICK_BUTTON_DOWN) {    
      break;
    }
    exit = menu_processKeyboard();
  }

  hw_waitVerticalBlank();
  custom->dmacon = DMAF_RASTER;
  palette_black();


  return exit;
}
