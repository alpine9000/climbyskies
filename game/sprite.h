#ifndef __SPRITE_H
#define __SPRITE_H


typedef struct {
  gfx_blit_t blit[2];
} sprite_save_t;  


typedef struct {
  int16_t x;
  int16_t y;
} velocity_t;


typedef struct {
  int16_t x;
  int16_t y;
  int16_t w;
  int16_t h;
} image_t;


#ifdef PLAYER_COLLISION_BOX
typedef struct {
  int16_t x1;
  int16_t x2;
  int16_t y1;
  int16_t y2;
} collision_coords_t;
#endif

typedef struct {
  int16_t x;
  int16_t y;
  int16_t imageIndex;
  sprite_save_t* save;
  image_t* image;
#ifdef PLAYER_COLLISION_BOX
  collision_coords_t collisionBox;
#endif
} sprite_t;

#ifdef PLAYER_HARDWARE_SPRITE
typedef struct {
  uint8_t vStartLo;
  uint8_t hStartHi;
  uint8_t vStopLo;
  uint8_t attach:1;
  uint8_t unused:4;
  uint8_t vStartHi:1;
  uint8_t vStopHi:1;
  uint8_t hStartLow:1;
} sprite_hsprite_control_t;

typedef struct {
  sprite_hsprite_control_t* hsprite00;
  sprite_hsprite_control_t* hsprite01;
  sprite_hsprite_control_t* hsprite10;
  sprite_hsprite_control_t* hsprite11;
} sprite_hsprite_t;

extern sprite_hsprite_t sprite_hspriteAtlas[];
#endif
extern image_t sprite_imageAtlas[];

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

void
sprite_ctor(void);

#define sprite_render(fb, a) _sprite_render(fb, &a, gfx_renderSprite)
#define sprite_render16NoShift(fb, a) _sprite_render(fb, &a, gfx_renderSprite16NoShift)

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
  SPRITE_CLOUD_2 = 13,
  SPRITE_ENEMY_RUN_LEFT_1 = 14,
  SPRITE_ENEMY_RUN_LEFT_2 = 15,
  SPRITE_ENEMY_RUN_LEFT_3 = 16,
  SPRITE_ENEMY_RUN_LEFT_4 = 17,
  SPRITE_ENEMY_RUN_RIGHT_1 = 18,
  SPRITE_ENEMY_RUN_RIGHT_2 = 19,
  SPRITE_ENEMY_RUN_RIGHT_3 = 20,
  SPRITE_ENEMY_RUN_RIGHT_4 = 21,
  SPRITE_ENEMY_SKATE_RIGHT_1 = 22,
  SPRITE_ENEMY_SKATE_RIGHT_2 = 23,
  SPRITE_ENEMY_SKATE_LEFT_1 = 24,
  SPRITE_ENEMY_SKATE_LEFT_2 = 25,
  SPRITE_ENEMY_DRAGON_RIGHT_1 = 26,
  SPRITE_ENEMY_DRAGON_RIGHT_2 = 27,
  SPRITE_ENEMY_DRAGON_RIGHT_3 = 28,
  SPRITE_ENEMY_DRAGON_RIGHT_4 = 29,
  SPRITE_COIN_1 = 30,
  SPRITE_COIN_2 = 31,
  SPRITE_COIN_3 = 32,
  SPRITE_COIN_4 = 33,
  SPRITE_COIN_5 = 34,
  SPRITE_COIN_6 = 35,
  SPRITE_COIN_7 = 36,
  SPRITE_COIN_8 = 37,
  SPRITE_COIN_9 = 38,
  SPRITE_COIN_10 = 39,
  SPRITE_COIN_11 = 40,
  SPRITE_COIN_12 = 41,
  SPRITE_COIN_13 = 42,
  SPRITE_COIN_14 = 43,
} sprite_id_t;


extern uint16_t sprite_playerLeftRun0_0_sprite0[];
extern uint16_t sprite_playerLeftRun0_0_sprite1[];
extern uint16_t sprite_playerLeftRun1_0_sprite0[];
extern uint16_t sprite_playerLeftRun1_0_sprite1[];

extern uint16_t sprite_playerLeftRun0_1_sprite0[];
extern uint16_t sprite_playerLeftRun0_1_sprite1[];
extern uint16_t sprite_playerLeftRun1_1_sprite0[];
extern uint16_t sprite_playerLeftRun1_1_sprite1[];

extern uint16_t sprite_playerLeftRun0_2_sprite0[];
extern uint16_t sprite_playerLeftRun0_2_sprite1[];
extern uint16_t sprite_playerLeftRun1_2_sprite0[];
extern uint16_t sprite_playerLeftRun1_2_sprite1[];

extern uint16_t sprite_playerLeftRun0_3_sprite0[];
extern uint16_t sprite_playerLeftRun0_3_sprite1[];
extern uint16_t sprite_playerLeftRun1_3_sprite0[];
extern uint16_t sprite_playerLeftRun1_3_sprite1[];



extern uint16_t sprite_playerRightRun0_0_sprite0[];
extern uint16_t sprite_playerRightRun0_0_sprite1[];
extern uint16_t sprite_playerRightRun1_0_sprite0[];
extern uint16_t sprite_playerRightRun1_0_sprite1[];

extern uint16_t sprite_playerRightRun0_1_sprite0[];
extern uint16_t sprite_playerRightRun0_1_sprite1[];
extern uint16_t sprite_playerRightRun1_1_sprite0[];
extern uint16_t sprite_playerRightRun1_1_sprite1[];

extern uint16_t sprite_playerRightRun0_2_sprite0[];
extern uint16_t sprite_playerRightRun0_2_sprite1[];
extern uint16_t sprite_playerRightRun1_2_sprite0[];
extern uint16_t sprite_playerRightRun1_2_sprite1[];

extern uint16_t sprite_playerRightRun0_3_sprite0[];
extern uint16_t sprite_playerRightRun0_3_sprite1[];
extern uint16_t sprite_playerRightRun1_3_sprite0[];
extern uint16_t sprite_playerRightRun1_3_sprite1[];


extern uint16_t sprite_playerRightJump0_0_sprite0[];
extern uint16_t sprite_playerRightJump0_0_sprite1[];
extern uint16_t sprite_playerRightJump1_0_sprite0[];
extern uint16_t sprite_playerRightJump1_0_sprite1[];

extern uint16_t sprite_playerLeftJump0_0_sprite0[];
extern uint16_t sprite_playerLeftJump0_0_sprite1[];
extern uint16_t sprite_playerLeftJump1_0_sprite0[];
extern uint16_t sprite_playerLeftJump1_0_sprite1[];

extern uint16_t sprite_playerRightStand0_0_sprite0[];
extern uint16_t sprite_playerRightStand0_0_sprite1[];
extern uint16_t sprite_playerRightStand1_0_sprite0[];
extern uint16_t sprite_playerRightStand1_0_sprite1[];

extern uint16_t sprite_playerLeftStand0_0_sprite0[];
extern uint16_t sprite_playerLeftStand0_0_sprite1[];
extern uint16_t sprite_playerLeftStand1_0_sprite0[];
extern uint16_t sprite_playerLeftStand1_0_sprite1[];

extern uint16_t sprite_nullhsprite[];

#endif
