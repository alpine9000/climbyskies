#ifndef __SPRITE_H
#define __SPRITE_H


typedef struct {
  gfx_blit_t blit[2];
} sprite_save_t;  


typedef struct {
  int x;
  int y;
} velocity_t;

typedef struct {
  int x;
  int y;
  int imageIndex;
  sprite_save_t* save;
} sprite_t;

typedef struct {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} image_t;

extern image_t imageAtlas[];

#ifndef INLINE_EVERYTHING
void
_sprite_render(frame_buffer_t fb, sprite_t* actor, void (*render)(frame_buffer_t dest, int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h));
void 
sprite_save(frame_buffer_t fb, sprite_t* a);
void
sprite_restore(sprite_save_t* save);
#else
#include "sprite_inlines.h"
#endif

#define sprite_render(fb, a) _sprite_render(fb, &a, gfx_renderSprite)
#define sprite_renderNoMask(fb, a) _sprite_render(fb, &a, gfx_renderSpriteNoMask)

typedef enum  {
  SPRITE_CLIMBER_RUN_LEFT_1 = 0,
  SPRITE_CLIMBER_RUN_LEFT_2 = 1,
  SPRITE_CLIMBER_RUN_LEFT_3 = 2,
  SPRITE_CLIMBER_RUN_LEFT_4 = 3,
  SPRITE_CLIMBER_STAND_LEFT = 4,
  SPRITE_CLIMBER_JUMP_LEFT = 5,
  SPRITE_CLIMBER_RUN_RIGHT_1 = 6,
  SPRITE_CLIMBER_RUN_RIGHT_2 = 7,
  SPRITE_CLIMBER_RUN_RIGHT_3 = 8,
  SPRITE_CLIMBER_RUN_RIGHT_4 = 9,
  SPRITE_CLIMBER_STAND_RIGHT = 10,
  SPRITE_CLIMBER_JUMP_RIGHT = 11,
  SPRITE_CLOUD_1 = 12,
  SPRITE_CLOUD_2 = 13
} sprite_id_t;


#endif
