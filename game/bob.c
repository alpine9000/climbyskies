#include "game.h"

typedef struct {
  uint16_t x;
  uint16_t y;
  uint16_t w;
  uint16_t h;
} bob_t;

bob_t bobs[] = {
  { // climber run left 1
    .x = 0,
    .y = 80,
    .w = 32,
    .h = 48
  },
  { // climber run left 2
    .x = 32,
    .y = 80,
    .w = 32,
    .h = 48
  },
  { // climber run left 3
    .x = 64,
    .y = 80,
    .w = 32,
    .h = 48
  },
  { // climber run left 4
    .x = 96,
    .y = 80,
    .w = 32,
    .h = 48
  },
  { // climber stand left 
    .x = 128,
    .y = 80,
    .w = 32,
    .h = 48
  },
  { // climber jump left
    .x = 160,
    .y = 80,
    .w = 32,
    .h = 48
  },
  { // climber run right 1
    .x = 0,
    .y = 128,
    .w = 32,
    .h = 48
  },
  { // climber run right 2
    .x = 32,
    .y = 128,
    .w = 32,
    .h = 48
  },
  { // climber run right 3
    .x = 64,
    .y = 128,
    .w = 32,
    .h = 48
  },
  { // climber run right 4
    .x = 96,
    .y = 128,
    .w = 32,
    .h = 48
  },
  { // climber stand right 
    .x = 128,
    .y = 128,
    .w = 32,
    .h = 48
  },
};

void
bob_render(frame_buffer_t fb, int16_t x, int16_t y, uint16_t b)
{
  bob_t* bob = &bobs[b];
  gfx_renderSprite(fb, bob->x, bob->y, x, y-cameraY-screenScrollY, bob->w, bob->h);
}

void
bob_clear(frame_buffer_t fb, int16_t x, int16_t y, uint16_t b)
{
  bob_t* bob = &bobs[b];
  gfx_clearSprite(fb, x, y-cameraY-screenScrollY, bob->w, bob->h);
}
