#include "game.h"

extern void palette_menuInstall(void);
extern frame_buffer_t menuFrameBuffer;

static  __section(data_c)  copper_t menu_copper  = {
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

int16_t
menu_loop(void)
{
  volatile uint16_t scratch;

  hw_waitVerticalBlank();
  palette_black();

  disk_loadData((void*)game_onScreenBuffer, (void*)menuFrameBuffer, SCREEN_WIDTH_BYTES*SCREEN_HEIGHT*SCREEN_BIT_DEPTH);

  uint16_t volatile* copperPtr = (uint16_t*)&menu_copper;

  custom->dmacon = (DMAF_SPRITE); // turn off sprite dma

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


  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_COPPER|DMAF_RASTER|DMAF_MASTER);

  hw_waitVerticalBlank();
  palette_menuFadeIn();

  int exit = 0;

  while (!exit) {
    hw_readJoystick();
    if (JOYSTICK_BUTTON_DOWN) {    
      break;
    }
    exit = menu_processKeyboard();
  }

  palette_black();

  return exit;
}
