INLINE void
sprite_save(frame_buffer_t fb, sprite_t* a)
{
  image_t* image = &imageAtlas[a->imageIndex];
  int h = image->h;
  int y = a->y;
  if (y < cameraY) {
    h -= (cameraY - y);
    y += (cameraY - y);
  }

  if (y-cameraY + h > SCREEN_HEIGHT) {
    h -= (y-cameraY+h)-SCREEN_HEIGHT;
  }

  if (h <= 0) {
    a->save->blit[0].size = 0;
    a->save->blit[1].size = 0;
    return;
  }
  y = y-cameraY-screenScrollY;
  if (y >= 0) {
    gfx_saveSprite(fb, &a->save->blit[0], a->x, y, image->w, h);
    a->save->blit[1].size = 0;
  } else {
    if (y > -h) {
      gfx_saveSprite(fb, &a->save->blit[0], a->x, 0, image->w, h+y);    
      gfx_saveSprite(fb, &a->save->blit[1], a->x, FRAME_BUFFER_HEIGHT+y, image->w, -y);    
    } else {
      gfx_saveSprite(fb, &a->save->blit[0], a->x, FRAME_BUFFER_HEIGHT+y, image->w, h);    
      a->save->blit[1].size = 0;
    }
  }
}


INLINE void
_sprite_render(frame_buffer_t fb, sprite_t* sprite, void (*render)(frame_buffer_t dest, int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h))
{
  image_t* image = &imageAtlas[sprite->imageIndex];
  int by = image->y;
  int h = image->h;
  int y = sprite->y;
  if (y < cameraY) {
    h -= (cameraY - y);
    by += (cameraY - y);
    y += (cameraY - y);
  }

  if (y-cameraY + h > SCREEN_HEIGHT) {
    h -= (y-cameraY+h)-SCREEN_HEIGHT;
  }

    if (h <= 0) {
      return;
    }
    
  y = y-cameraY-screenScrollY;
  if (y >= 0) {
    (*render)(fb, image->x, by, sprite->x, y, image->w, h);
  } else {
    if (y > -h) {
      (*render)(fb, image->x, by-y, sprite->x, 0, image->w, h+y);    
      (*render)(fb, image->x, by, sprite->x, FRAME_BUFFER_HEIGHT+y, image->w, -y);    
    } else {
      (*render)(fb, image->x, by, sprite->x, FRAME_BUFFER_HEIGHT+y, image->w, h);    
    }
  }
}


INLINE void
sprite_restore(sprite_save_t* save)
{
  if (save->blit[0].size > 0) {
    gfx_restoreSprite(&save->blit[0]);
  }

  if (save->blit[1].size > 0) {
    gfx_restoreSprite(&save->blit[1]);
  }
}

