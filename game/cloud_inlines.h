
#ifdef CLOUD_ENABLE_SETUPRENDERPARTIALTILE
static inline 
void
cloud_setupRenderPartialTile(void)
{
  volatile struct Custom* _custom = CUSTOM;
  hw_waitBlitter();

  _custom->bltamod = FRAME_BUFFER_WIDTH_BYTES-2;
  _custom->bltdmod = FRAME_BUFFER_WIDTH_BYTES-2;
}
#endif

static inline void
cloud_renderPartialTile(frame_buffer_t dest, int16_t x, int16_t y, uint16_t h, frame_buffer_t tile)
{
  volatile struct Custom* _custom = CUSTOM;
  if (y < 0) {
    return;
  }
  dest += gfx_dyOffsetsLUT[y] + (x>>3);

  hw_waitBlitter();

#ifndef CLOUD_ENABLE_SETUPRENDERPARTIALTILE
  _custom->bltamod = FRAME_BUFFER_WIDTH_BYTES-2;
  _custom->bltdmod = FRAME_BUFFER_WIDTH_BYTES-2;
#endif

  _custom->bltapt = (uint8_t*)tile;
  _custom->bltdpt = (uint8_t*)dest;
  _custom->bltsize = gfx_heightLUT[h] | 1;
}


static inline void
cloud_renderTile(frame_buffer_t fb, int16_t x, int16_t y, frame_buffer_t tile)
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
    cloud_renderPartialTile(fb, x, y, h, tile);
  } else {
    if (y > -h) {
      cloud_renderPartialTile(fb, x, y, h+y, tile);
      cloud_renderPartialTile(fb, x, FRAME_BUFFER_HEIGHT+y, -y, tile);
    } else {
      cloud_renderPartialTile(fb, x, FRAME_BUFFER_HEIGHT+y, h, tile);
    }
  }
}


static inline void
cloud_setupRenderSpriteNoMask(void)
{
  volatile struct Custom* _custom = CUSTOM;
  
  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|DEST|0xf0/*|shift<<ASHIFTSHIFT*/);
  _custom->bltcon1 = 0; //shift<<BSHIFTSHIFT;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;
#ifdef CLOUD_FULLCOLOR
  _custom->bltamod = (FRAME_BUFFER_WIDTH_BYTES-((CLOUD_WIDTH/16)<<1));
  _custom->bltdmod = (FRAME_BUFFER_WIDTH_BYTES-((CLOUD_WIDTH/16)<<1));
#else
  _custom->bltamod = (FRAME_BUFFER_WIDTH_BYTES-((CLOUD_WIDTH/16)<<1))+(FRAME_BUFFER_WIDTH_BYTES*(SCREEN_BIT_DEPTH-1));
  _custom->bltdmod = (FRAME_BUFFER_WIDTH_BYTES-((CLOUD_WIDTH/16)<<1))+(FRAME_BUFFER_WIDTH_BYTES*(SCREEN_BIT_DEPTH-1));
#endif
}


static inline void
cloud_renderSpriteNoMask(frame_buffer_t dest, int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t h)
{
  volatile struct Custom* _custom = CUSTOM;
  frame_buffer_t source = spriteFrameBuffer;
  
  dest += gfx_dyOffsetsLUT[dy] + (dx>>3);
  source += gfx_dyOffsetsLUT[sy] + (sx>>3);

#ifdef CLOUD_FULLCOLOR
  hw_waitBlitter();

  _custom->bltapt = (uint8_t*)source;
  _custom->bltdpt = (uint8_t*)dest;
  _custom->bltsize = gfx_heightLUT[h] | (CLOUD_WIDTH/16);
#else

  source += FRAME_BUFFER_WIDTH_BYTES;
  dest += FRAME_BUFFER_WIDTH_BYTES;

  hw_waitBlitter();

  _custom->bltapt = (uint8_t*)source;
  _custom->bltdpt = (uint8_t*)dest;
  //  _custom->bltsize = gfx_heightLUT[h] | (CLOUD_WIDTH/16);
  _custom->bltsize = cloud_sizeLUT[h];

  source += FRAME_BUFFER_WIDTH_BYTES;
  dest += FRAME_BUFFER_WIDTH_BYTES;

  hw_waitBlitter();

  _custom->bltapt = (uint8_t*)source;
  _custom->bltdpt = (uint8_t*)dest;
  //  _custom->bltsize = gfx_heightLUT[h] | (CLOUD_WIDTH/16);
  _custom->bltsize = cloud_sizeLUT[h];

#endif

}


static inline void
cloud_renderSpriteNoMaskDefaultHeight(frame_buffer_t dest, int16_t sx, int16_t sy, int16_t dx, int16_t dy)
{
  volatile struct Custom* _custom = CUSTOM;
  frame_buffer_t source = spriteFrameBuffer;
  
  dest += gfx_dyOffsetsLUT[dy] + (dx>>3);
  source += gfx_dyOffsetsLUT[sy] + (sx>>3);

  hw_waitBlitter();

  _custom->bltapt = (uint8_t*)source;
  _custom->bltdpt = (uint8_t*)dest;
  _custom->bltsize = ((CLOUD_HEIGHT*SCREEN_BIT_DEPTH)<<6) | (CLOUD_WIDTH/16);

}


static inline void
cloud_spriteRender(frame_buffer_t fb, sprite_t* sprite)
{
  image_t* image = &sprite_imageAtlas[sprite->imageIndex];
  int by = image->y;
  int h = image->h;
  int y = sprite->y;
  if (y < game_cameraY) {
    h -= (game_cameraY - y);
    by += (game_cameraY - y);
    y += (game_cameraY - y);
  }

  if (y-game_cameraY + h > SCREEN_HEIGHT) {
    h -= (y-game_cameraY+h)-SCREEN_HEIGHT;
  }

  if (h <= 0) {
    return;
  }
    
  y = y-game_cameraY-game_screenScrollY;
  if (y >= 0) {
    cloud_renderSpriteNoMask(fb, image->x, by, sprite->x, y, h);
  } else {
    if (y > -h) {
      cloud_renderSpriteNoMask(fb, image->x, by-y, sprite->x, 0, h+y);    
      cloud_renderSpriteNoMask(fb, image->x, by, sprite->x, FRAME_BUFFER_HEIGHT+y, -y);    
    } else {
      cloud_renderSpriteNoMask(fb, image->x, by, sprite->x, FRAME_BUFFER_HEIGHT+y, h);
    }
  }
}


static inline void
cloud_saveSprite(frame_buffer_t source, gfx_blit_t* blit, int16_t dx, int16_t dy, int16_t w, int16_t h)
{
  volatile struct Custom* _custom = CUSTOM;
  blit->dest = game_saveBuffer;
  //  uint32_t widthWords =  ((w+15)>>4)+1;
  USE(w);
  uint32_t widthWords =  CLOUD_WIDTH_WORDS;//((w+15)>>4)+1;

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


static inline void
cloud_save(frame_buffer_t fb, sprite_t* a)
{
  image_t* image = &sprite_imageAtlas[a->imageIndex];
  int h = image->h;
  int y = a->y;
  if (y < game_cameraY) {
    h -= (game_cameraY - y);
    y += (game_cameraY - y);
  }

  if (y-game_cameraY + h > SCREEN_HEIGHT) {
    h -= (y-game_cameraY+h)-SCREEN_HEIGHT;
  }

  if (h <= 0) {
    a->save->blit[0].size = 0;
    a->save->blit[1].size = 0;
    return;
  }
  y = y-game_cameraY-game_screenScrollY;
  if (y >= 0) {
    cloud_saveSprite(fb, &a->save->blit[0], a->x, y, image->w, h);
    a->save->blit[1].size = 0;
  } else {
    if (y > -h) {
      cloud_saveSprite(fb, &a->save->blit[0], a->x, 0, image->w, h+y);    
      cloud_saveSprite(fb, &a->save->blit[1], a->x, FRAME_BUFFER_HEIGHT+y, image->w, -y);    
    } else {
      cloud_saveSprite(fb, &a->save->blit[0], a->x, FRAME_BUFFER_HEIGHT+y, image->w, h);    
      a->save->blit[1].size = 0;
    }
  }
}
