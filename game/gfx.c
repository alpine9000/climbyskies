#include "game.h"

__EXTERNAL uint16_t gfx_dyOffsetsLUT[FRAME_BUFFER_HEIGHT+1];
uint16_t gfx_heightLUT[65];
uint16_t gfx_renderSprite16NoShiftSetup;

void 
gfx_ctor()
{
  for (uint16_t y = 0; y <= FRAME_BUFFER_HEIGHT; y++) {
    gfx_dyOffsetsLUT[y] = (y * (FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH));
  }

  for (uint16_t h = 0; h <= 64; h++) {
    gfx_heightLUT[h] = (h*SCREEN_BIT_DEPTH)<<6;
  }
}

#ifndef INLINE_EVERYTHING
#include "gfx_inlines.h"
#endif
