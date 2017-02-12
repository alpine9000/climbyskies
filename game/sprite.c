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
    .y = 173,
    .w = 32,
    .h = ENEMY_RUN_HEIGHT
  },
  [SPRITE_ENEMY_RUN_LEFT_2] = { 
    .x = 32,
    .y = 173,
    .w = 32,
    .h = ENEMY_RUN_HEIGHT
  },
  [SPRITE_ENEMY_RUN_LEFT_3] = { 
    .x = 64,
    .y = 173,
    .w = 32,
    .h = ENEMY_RUN_HEIGHT
  },
  [SPRITE_ENEMY_RUN_LEFT_4] = { 
    .x = 96,
    .y = 173,
    .w = 32,
    .h = ENEMY_RUN_HEIGHT
  },
  [SPRITE_ENEMY_RUN_RIGHT_1] = { 
    .x = 0,
    .y = 221,
    .w = 32,
    .h = ENEMY_RUN_HEIGHT
  },
  [SPRITE_ENEMY_RUN_RIGHT_2] = { 
    .x = 32,
    .y = 221,
    .w = 32,
    .h = ENEMY_RUN_HEIGHT
  },
  [SPRITE_ENEMY_RUN_RIGHT_3] = { 
    .x = 64,
    .y = 221,
    .w = 32,
    .h = ENEMY_RUN_HEIGHT
  },
  [SPRITE_ENEMY_RUN_RIGHT_4] = { 
    .x = 96,
    .y = 221,
    .w = 32,
    .h = ENEMY_RUN_HEIGHT
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
    .y = 219,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_SKATE_LEFT_2] = { 
    .x = 160,
    .y = 219,
    .w = 32,
    .h = 37
  },
  [SPRITE_ENEMY_DRAGON_RIGHT_1] = { 
    .x = 96,
    .y = 38,
    .w = 32,
    .h = 25
  },
  [SPRITE_ENEMY_DRAGON_RIGHT_2] = { 
    .x = 128,
    .y = 38,
    .w = 32,
    .h = 25
  },
  [SPRITE_ENEMY_DRAGON_RIGHT_3] = { 
    .x = 160,
    .y = 38,
    .w = 32,
    .h = 25
  },
  [SPRITE_ENEMY_DRAGON_RIGHT_4] = { 
    .x = 192,
    .y = 38,
    .w = 32,
    .h = 25
  },
  [SPRITE_COIN_1] = { 
    .x = 96,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_2] = { 
    .x = 112,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_3] = { 
    .x = 128,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_4] = { 
    .x = 144,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_5] = { 
    .x = 160,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_6] = { 
    .x = 176,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_7] = { 
    .x = 192,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_8] = { 
    .x = 208,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_9] = { 
    .x = 224,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_10] = { 
    .x = 240,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_11] = { 
    .x = 256,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_12] = { 
    .x = 272,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_13] = { 
    .x = 288,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },
  [SPRITE_COIN_14] = { 
    .x = 304,
    .y = 2,
    .w = ITEM_WIDTH,
    .h = ITEM_HEIGHT
  },



};


#ifndef INLINE_EVERYTHING
#include "sprite_inlines.h"
#endif
