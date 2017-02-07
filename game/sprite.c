#include "game.h"



//static
image_t imageAtlas[] = {
  [SPRITE_CLIMBER_RUN_LEFT_1] = { // 0. climber run left 1
    .x = 0,
    .y = 91,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_LEFT_2] = { // 1. climber run left 2
    .x = 32,
    .y = 91,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_LEFT_3] = { // 2. climber run left 3
    .x = 64,
    .y = 91,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_LEFT_4] = { // 3. climber run left 4
    .x = 96,
    .y = 91,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_STAND_LEFT] =  { // 4. climber stand left 
    .x = 139,
    .y = 91,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_JUMP_LEFT] =  { // 5. climber jump left
    .x = 160,
    .y = 91,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_RIGHT_1] = { // 6. climber run right 1
    .x = 0,
    .y = 139,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_RIGHT_2] = { // 7. climber run right 2
    .x = 32,
    .y = 139,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_RIGHT_3] = { // 8. climber run right 3
    .x = 64,
    .y = 139,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_RIGHT_4] = { // 9. climber run right 4
    .x = 96,
    .y = 139,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_STAND_RIGHT] = { // 10. climber stand right 
    .x = 128,
    .y = 139,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_JUMP_RIGHT]  = { // 11. climber jump right
    .x = 160,
    .y = 139,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLOUD_1]  = {
    .x = 0,
    .y = 32,
    .w = CLOUD_WIDTH,
    .h = CLOUD_HEIGHT
  },
  [SPRITE_CLOUD_2]  = {
    .x = 48,
    .y = 32,
    .w = CLOUD_WIDTH,
    .h = CLOUD_HEIGHT
  },
};


void
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


void
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


void
sprite_restore(sprite_save_t* save)
{
  if (save->blit[0].size > 0) {
    gfx_restoreSprite(&save->blit[0]);
  }

  if (save->blit[1].size > 0) {
    gfx_restoreSprite(&save->blit[1]);
  }
}

