#include "game.h"

image_t sprite_imageAtlas[] = {
  [SPRITE_CLIMBER_RUN_LEFT_1] = { // 0. climber run left 1
    .x = 0,
    .y = 75,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_LEFT_2] = { // 1. climber run left 2
    .x = 32,
    .y = 75,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_LEFT_3] = { // 2. climber run left 3
    .x = 64,
    .y = 75,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_LEFT_4] = { // 3. climber run left 4
    .x = 96,
    .y = 75,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_STAND_LEFT] =  { // 4. climber stand left 
    .x = 139,
    .y = 75,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_JUMP_LEFT] =  { // 5. climber jump left
    .x = 160,
    .y = 75,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_RIGHT_1] = { // 6. climber run right 1
    .x = 0,
    .y = 123,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_RIGHT_2] = { // 7. climber run right 2
    .x = 32,
    .y = 123,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_RIGHT_3] = { // 8. climber run right 3
    .x = 64,
    .y = 123,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_RUN_RIGHT_4] = { // 9. climber run right 4
    .x = 96,
    .y = 123,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_STAND_RIGHT] = { // 10. climber stand right 
    .x = 128,
    .y = 123,
    .w = 32,
    .h = 37
  },
  [SPRITE_CLIMBER_JUMP_RIGHT]  = { // 11. climber jump right
    .x = 160,
    .y = 123,
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
  [SPRITE_ENEMY_RUN_LEFT_1] = { 
    .x = 0,
    .y = 171,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_RUN_LEFT_2] = { 
    .x = 32,
    .y = 171,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_RUN_LEFT_3] = { 
    .x = 64,
    .y = 171,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_RUN_LEFT_4] = { 
    .x = 96,
    .y = 171,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_RUN_RIGHT_1] = { 
    .x = 0,
    .y = 219,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_RUN_RIGHT_2] = { 
    .x = 32,
    .y = 219,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_RUN_RIGHT_3] = { 
    .x = 64,
    .y = 219,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_RUN_RIGHT_4] = { 
    .x = 96,
    .y = 219,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_SKATE_RIGHT_1] = { 
    .x = 128,
    .y = 171,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_SKATE_RIGHT_2] = { 
    .x = 160,
    .y = 171,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_SKATE_LEFT_1] = { 
    .x = 128,
    .y = 218,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_SKATE_LEFT_2] = { 
    .x = 160,
    .y = 219,
    .w = 32,
    .h = 37
  },
};


#ifndef INLINE_EVERYTHING
#include "sprite_inlines.h"
#endif
