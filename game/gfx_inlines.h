INLINE void
gfx_fillRect(frame_buffer_t fb, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color)
{
  static custom_t _custom = CUSTOM;
  static uint16_t startBitPatterns[] = { 0xffff,
			       0x7fff, 0x3fff, 0x1fff, 0x0fff, 
			       0x07ff, 0x03ff, 0x01ff, 0x00ff,
			       0x007f, 0x003f, 0x001f, 0x000f,
			       0x0007, 0x0003, 0x0001, 0x0000 };

  static uint16_t endBitPatterns[] = { 0xffff, 
				    0x8000, 0xc000, 0xe000, 0xf000,
				    0xf800, 0xfc00, 0xfe00, 0xff00,
				    0xff80, 0xffc0, 0xffe0, 0xfff0,
				    0xfff8, 0xfffc, 0xfffe, 0xffff};

  uint16_t startMask = startBitPatterns[x & 0xf]; 
  uint16_t endMask = endBitPatterns[(x+w) & 0xf]; 
  uint32_t widthWords = (((x&0x0f)+w)+15)>>4;
  
  if (widthWords == 1) {
    startMask &= endMask;
  }
  
  fb += gfx_dyOffsetsLUT[y] + (x>>3);

  int colorInPlane;
  for (int plane = 0; plane < SCREEN_BIT_DEPTH; plane++) {
    colorInPlane = (1<<plane) & color;
    hw_waitBlitter();
    
    _custom->bltcon0 = (SRCC|DEST|0xca);
    _custom->bltcon1 = 0;
    _custom->bltafwm = 0xffff;
    _custom->bltalwm = 0xffff;
    _custom->bltdmod = (FRAME_BUFFER_WIDTH_BYTES*(SCREEN_BIT_DEPTH-1))+(FRAME_BUFFER_WIDTH_BYTES-2);
    _custom->bltcmod = (FRAME_BUFFER_WIDTH_BYTES*(SCREEN_BIT_DEPTH-1))+(FRAME_BUFFER_WIDTH_BYTES-2);
    _custom->bltbmod = 0;
    _custom->bltamod = 0;
    _custom->bltadat = startMask;
    _custom->bltbdat = colorInPlane ? 0xffff : 0x0;
    _custom->bltcpt = (uint8_t*)fb;
    _custom->bltdpt = (uint8_t*)fb;
    _custom->bltsize = h<<6 | 1;
    
    if (widthWords > 1) {
      hw_waitBlitter();    
      _custom->bltcon0 = (SRCC|DEST|0xca);
      _custom->bltadat = endMask;
      _custom->bltcpt = (uint8_t*)fb+((widthWords-1)<<1);
      _custom->bltdpt = (uint8_t*)fb+((widthWords-1)<<1);
      _custom->bltsize = h<<6 | 1;
    }
    
    if (widthWords > 2) {
      hw_waitBlitter();    
      _custom->bltcon0 = (DEST|(colorInPlane ? 0xff : 0x00));
      _custom->bltdmod = (FRAME_BUFFER_WIDTH_BYTES*(SCREEN_BIT_DEPTH-1))+(FRAME_BUFFER_WIDTH_BYTES-((widthWords-2)<<1));
      _custom->bltdpt = (uint8_t*)fb+2;
      _custom->bltsize = h<<6 | (widthWords-2);
    }    

    fb += FRAME_BUFFER_WIDTH_BYTES;
  }
}

INLINE void
gfx_renderSprite(frame_buffer_t dest, int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h)
{
  static volatile struct Custom* _custom = CUSTOM;
  frame_buffer_t source = spriteFrameBuffer;
  frame_buffer_t mask = spriteMask;
  uint32_t widthWords =  ((w+15)>>4)+1;
  int shift = (dx&0xf);
  
  dest += gfx_dyOffsetsLUT[dy] + (dx>>3);
  source += gfx_dyOffsetsLUT[sy] + (sx>>3);
  mask += gfx_dyOffsetsLUT[sy] + (sx>>3);

  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|SRCB|SRCC|DEST|0xca|shift<<ASHIFTSHIFT);
  _custom->bltcon1 = shift<<BSHIFTSHIFT;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0x0000;
  _custom->bltamod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltbmod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltcmod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltdmod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltapt = (uint8_t*)mask;
  _custom->bltbpt = (uint8_t*)source;
  _custom->bltcpt = (uint8_t*)dest;
  _custom->bltdpt = (uint8_t*)dest;
  //  _custom->bltsize = (h*SCREEN_BIT_DEPTH)<<6 | widthWords;
  _custom->bltsize = gfx_heightLUT[h] | widthWords;

}

INLINE void
gfx_renderSprite16NoShift(frame_buffer_t dest, int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h)
{
  USE(w);
  static volatile struct Custom* _custom = CUSTOM;
  frame_buffer_t source = spriteFrameBuffer;
  frame_buffer_t mask = spriteMask;
  const uint32_t widthWords =  1;
  
  dest += gfx_dyOffsetsLUT[dy] + (dx>>3);
  source += gfx_dyOffsetsLUT[sy] + (sx>>3);
  mask += gfx_dyOffsetsLUT[sy] + (sx>>3);

  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|SRCB|SRCC|DEST|0xca|0);
  _custom->bltcon1 = 0;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;
  _custom->bltamod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltbmod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltcmod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltdmod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltapt = (uint8_t*)mask;
  _custom->bltbpt = (uint8_t*)source;
  _custom->bltcpt = (uint8_t*)dest;
  _custom->bltdpt = (uint8_t*)dest;
  //  _custom->bltsize = (h*SCREEN_BIT_DEPTH)<<6 | widthWords;
  _custom->bltsize = gfx_heightLUT[h] | widthWords;

}


INLINE void
gfx_renderSpriteNoMask(frame_buffer_t dest, int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h)
{
  static volatile struct Custom* _custom = CUSTOM;
  frame_buffer_t source = spriteFrameBuffer;
  uint32_t widthWords =  ((w+15)>>4);
  int shift = (dx&0xf);
  
  dest += gfx_dyOffsetsLUT[dy] + (dx>>3);
  source += gfx_dyOffsetsLUT[sy] + (sx>>3);

  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|DEST|0xf0|shift<<ASHIFTSHIFT);
  _custom->bltcon1 = shift<<BSHIFTSHIFT;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;
  _custom->bltamod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltdmod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));
  _custom->bltapt = (uint8_t*)source;
  _custom->bltdpt = (uint8_t*)dest;
  _custom->bltsize = gfx_heightLUT[h] | widthWords;

}

INLINE void
gfx_saveSprite(frame_buffer_t source, gfx_blit_t* blit, int16_t dx, int16_t dy, int16_t w, int16_t h)
{
  static volatile struct Custom* _custom = CUSTOM;
  blit->dest = game_saveBuffer;
  uint32_t widthWords =  ((w+15)>>4)+1;
  
  source += gfx_dyOffsetsLUT[dy] + (dx>>3);

  blit->dest += gfx_dyOffsetsLUT[dy] + (dx>>3);
  blit->source = source;
  //blit->size = (h*SCREEN_BIT_DEPTH)<<6 | widthWords;
  blit->size = gfx_heightLUT[h] | widthWords;
  blit->mod = (FRAME_BUFFER_WIDTH_BYTES-(widthWords<<1));

  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|DEST|0xf0);
  _custom->bltcon1 = 0;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;
  _custom->bltamod = blit->mod;
  _custom->bltdmod = blit->mod;
  _custom->bltapt = (uint8_t*)blit->source;
  _custom->bltdpt = (uint8_t*)blit->dest;
  _custom->bltsize = blit->size;
}

INLINE void
gfx_restoreSprite(gfx_blit_t* blit)
{
  static volatile struct Custom* _custom = CUSTOM;

  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|DEST|0xf0);
  _custom->bltcon1 = 0;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;
  _custom->bltamod = blit->mod;
  _custom->bltdmod = blit->mod;
  _custom->bltapt = (uint8_t*)blit->dest;
  _custom->bltdpt = (uint8_t*)blit->source;
  _custom->bltsize = blit->size;
}


INLINE void
gfx_renderTileOffScreen(frame_buffer_t dest, int16_t x, int16_t y, frame_buffer_t tile)
{
  static volatile struct Custom* _custom = CUSTOM;
  
  dest += gfx_dyOffsetsLUT[y] + (x>>3);

  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|DEST|0xf0);
  _custom->bltcon1 = 0;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;
  _custom->bltamod = FRAME_BUFFER_WIDTH_BYTES-2;
  _custom->bltdmod = FRAME_BUFFER_WIDTH_BYTES-2;
  _custom->bltapt = (uint8_t*)tile;
  _custom->bltdpt = (uint8_t*)dest;
  _custom->bltsize = (16*SCREEN_BIT_DEPTH)<<6 | 1;
}

STATIC_INLINE void
gfx_renderPartialTile(frame_buffer_t dest, int16_t x, int16_t y, uint16_t h, frame_buffer_t tile)
{
  static volatile struct Custom* _custom = CUSTOM;
  if (y < 0) {
    return;
  }
  dest += gfx_dyOffsetsLUT[y] + (x>>3);

  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|DEST|0xf0);
  _custom->bltcon1 = 0;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;
  _custom->bltamod = FRAME_BUFFER_WIDTH_BYTES-2;
  _custom->bltdmod = FRAME_BUFFER_WIDTH_BYTES-2;
  _custom->bltapt = (uint8_t*)tile;
  _custom->bltdpt = (uint8_t*)dest;
  _custom->bltsize = gfx_heightLUT[h] | 1;
}

INLINE void
gfx_renderTile(frame_buffer_t fb, int16_t x, int16_t y, frame_buffer_t tile)
{
  int h = 16;
  if (y < game_cameraY) {
    int offset = game_cameraY - y;
    h -= offset;
    y += offset;
    tile += gfx_dyOffsetsLUT[offset];
    if (h <= 0) {
      return;
    }
  }


  y = y-game_cameraY-game_screenScrollY;
  if (y >= 0) {
    gfx_renderPartialTile(fb, x, y, h, tile);
  } else {
    if (y > -h) {
      gfx_renderPartialTile(fb, x, y, h+y, tile);
      gfx_renderPartialTile(fb, x, FRAME_BUFFER_HEIGHT+y, -y, tile);
    } else {
      gfx_renderPartialTile(fb, x, FRAME_BUFFER_HEIGHT+y, h, tile);
    }
  }
}
