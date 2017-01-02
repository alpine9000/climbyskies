#include "game.h"



void 
screen_pokeCopperList(frame_buffer_t frameBuffer, uint16_t* copper)
{
  /* poke bitplane pointers into copper list */
  volatile uint16_t* copperPtr = copper;
  uint32_t bitplanesPtr = (uint32_t)frameBuffer;

  for (int i = 0; i < SCREEN_BIT_DEPTH; i++) {
    copperPtr[1] = (uint16_t)bitplanesPtr;
    copperPtr[3] = (uint16_t)(((uint32_t)bitplanesPtr)>>16);
    bitplanesPtr = bitplanesPtr + (SCREEN_WIDTH_BYTES);
    copperPtr = copperPtr + 4;
  }
}


void 
screen_setup(frame_buffer_t frameBuffer, uint16_t* copper)
{
  unsigned i;
  volatile uint16_t scratch;
  volatile uint16_t* copperPtr = copper;
  volatile uint32_t bitplanesPtr = (uint32_t)frameBuffer;

  /* set up playfield */
  
  custom->diwstrt = (RASTER_Y_START<<8)|RASTER_X_START;
  custom->diwstop = ((RASTER_Y_STOP-256)<<8)|(RASTER_X_STOP-256);
  custom->ddfstrt = (RASTER_X_START/2-SCREEN_RES);
  custom->ddfstop = (RASTER_X_START/2-SCREEN_RES)+(8*((SCREEN_WIDTH/16)-1));
  custom->bplcon0 = (SCREEN_BIT_DEPTH<<12)|0x200;
  custom->bpl1mod = SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH-SCREEN_WIDTH_BYTES;
  custom->bpl2mod = SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH-SCREEN_WIDTH_BYTES;

  screen_pokeCopperList(frameBuffer, copper);

  /* install copper list, then enable dma and selected interrupts */
  custom->cop1lc = (uint32_t)copper;
  scratch = custom->copjmp1;
  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_COPPER|DMAF_RASTER|DMAF_MASTER);
  //  custom->intena = (INTF_SETCLR|INTF_VERTB|INTF_INTEN);

}

