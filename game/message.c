#include "game.h"

#define MESSAGE_BOX_WIDTH 160

extern frame_buffer_t message_frameBuffer;
static uint16_t message_on = 0;

#if TRACKLOADER==1
typedef struct {
  uint16_t bpl1[SCREEN_BIT_DEPTH*2*2];
  uint16_t end[2];
} message_copper_t;

static  __section(data_c)  message_copper_t message_copper  = {
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
message_pokeCopperList(frame_buffer_t frameBuffer)
{
  uint16_t volatile* copperPtr = (uint16_t*)&message_copper;
  /* poke bitplane pointers into copper list */
  uint32_t bitplanesPtr = (uint32_t)frameBuffer;

  for (int16_t i = 0; i < SCREEN_BIT_DEPTH; i++) {
    copperPtr[1] = (uint16_t)bitplanesPtr;
    copperPtr[3] = (uint16_t)(((uint32_t)bitplanesPtr)>>16);
    bitplanesPtr = bitplanesPtr + (SCREEN_WIDTH_BYTES);
    copperPtr = copperPtr + 4;
  }
}
#endif


void
message_screenOn(char* message)
{
  if (message_on) {
    return;
  }

  hw_waitVerticalBlank();
  palette_black();

#if TRACKLOADER==1
  volatile uint16_t scratch;

  custom->dmacon = DMAF_RASTER|DMAF_SPRITE;
  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_MASTER);

  uint16_t volatile* copperPtr = (uint16_t*)&message_copper;

  gfx_fillRectSmallScreen(game_offScreenBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  text_drawMaskedText8Blitter(game_offScreenBuffer, message, (SCREEN_WIDTH/2)-(strlen(message)*4), (SCREEN_HEIGHT/2)+4);
  hw_waitBlitter();
  custom->bltafwm = 0xffff;

  /* set up playfield */
  
  custom->diwstrt = (RASTER_Y_START<<8)|RASTER_X_START;
  custom->diwstop = ((MENU_RASTER_Y_STOP-256)<<8)|(RASTER_X_STOP-256);

  custom->ddfstrt = (RASTER_X_START/2-SCREEN_RES);
  custom->ddfstop = (RASTER_X_START/2-SCREEN_RES)+(8*((SCREEN_WIDTH/16)-1));
  custom->bplcon0 = (SCREEN_BIT_DEPTH<<12)|0x200;
  custom->bpl1mod = (SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH)-SCREEN_WIDTH_BYTES;
  custom->bpl2mod = (SCREEN_WIDTH_BYTES*SCREEN_BIT_DEPTH)-SCREEN_WIDTH_BYTES;

  /* install copper list, then enable dma and selected interrupts */
  custom->cop1lc = (uint32_t)copperPtr;
  scratch = custom->copjmp1;

  USE(scratch);

  message_pokeCopperList(game_offScreenBuffer);

  custom->dmacon = (DMAF_BLITTER|DMAF_SETCLR|DMAF_COPPER|DMAF_RASTER|DMAF_MASTER);

  hw_waitVerticalBlank();
  custom->color[1] = 0xfff;
#else
  USE(message);
#endif

  message_on = 1;
}


void
message_screenOff(void)
{
#if TRACKLOADER==1
  hw_waitVerticalBlank();
  palette_black();

  gfx_fillRectSmallScreen(game_offScreenBuffer, 0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0);
  hw_waitBlitter();

  hw_waitVerticalBlank();
  screen_setup((uint16_t*)&copper);
#endif
  message_on = 0;
}


void
message_box(char* message)
{
  gfx_fillRect(message_frameBuffer, 4, 4, MESSAGE_BOX_WIDTH-8, 8, 0);

  frame_buffer_t ptr = message_frameBuffer;
  ptr += FRAME_BUFFER_WIDTH_BYTES;
  text_drawText8(ptr, message, (MESSAGE_BOX_WIDTH/2)-((strlen(message)/2)*8), 4);
  ptr += 3*FRAME_BUFFER_WIDTH_BYTES;
  text_drawText8(ptr, message, (MESSAGE_BOX_WIDTH/2)-((strlen(message)/2)*8), 4);

  hw_waitVerticalBlank();
  gfx_splitBlitNoMask(game_onScreenBuffer, message_frameBuffer, (SCREEN_WIDTH/2)-(MESSAGE_BOX_WIDTH/2), 48, 0, 0, MESSAGE_BOX_WIDTH, 16);

  gfx_splitBlitNoMask(game_offScreenBuffer, message_frameBuffer, (SCREEN_WIDTH/2)-(MESSAGE_BOX_WIDTH/2), 48, 0, 0, MESSAGE_BOX_WIDTH, 16);
}


void
message_alert(char* message)
{
  message_box(message);  
  hw_waitForJoystick();
}
