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


void 
gfx_bitBlitNoMask(frame_buffer_t dest, frame_buffer_t src, int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h)
{
  static volatile struct Custom* _custom = CUSTOM;
  uint32_t widthWords =  ((w)>>4);
  int32_t shift = 0;//(dx&0xf);
  
  dest += gfx_dyOffsetsLUT[dy] + (dx>>3);
  src += gfx_dyOffsetsLUT[sy] + (sx>>3);

  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|DEST|0xf0|shift<<ASHIFTSHIFT);
  _custom->bltcon1 = shift<<BSHIFTSHIFT;
  _custom->bltalwm = 0xffff;
  _custom->bltamod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltdmod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltapt = (uint8_t*)src;
  _custom->bltdpt = (uint8_t*)dest;
  _custom->bltsize = gfx_heightLUT[h] | widthWords;
}


void
gfx_splitBlitNoMask(frame_buffer_t dest, frame_buffer_t src, int16_t dx, int16_t dy, int16_t sx, int16_t sy, int16_t w, int16_t _h)
{
  int32_t by = sy;
  int32_t h = _h;
  int32_t y = dy;

  if (y < 0) {
    h -= (-y);
    by += (-y);
    y = 0;
  }

  if (h == 0) {
    return;
  }
  
  y = y-game_screenScrollY;
  if (y >= 0) {
    gfx_bitBlitNoMask(dest, src, sx, by, dx, y, w, h);
  } else {
    if (y > -h) {
      gfx_bitBlitNoMask(dest, src, sx, by-y, dx, 0, w, h+y);    
      gfx_bitBlitNoMask(dest, src, sx, by, dx, FRAME_BUFFER_HEIGHT+y, w, -y);    
    } else {
      gfx_bitBlitNoMask(dest, src, sx, by, dx, FRAME_BUFFER_HEIGHT+y, w, h);    
    }
  }
}

#ifndef INLINE_EVERYTHING
#include "gfx_inlines.h"
#endif
