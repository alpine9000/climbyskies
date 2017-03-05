#ifndef __PLAYER_H
#define __PLAYER_H

#define PLAYER_WIDTH                32
#define PLAYER_FUZZY_WIDTH          8
#define PLAYER_VISIBLE_WIDTH        (PLAYER_WIDTH-PLAYER_FUZZY_WIDTH)
#define PLAYER_HEIGHT               37
#define PLAYER_BASE_PLATFORM_HEIGHT (TILE_HEIGHT*3)


typedef enum {
  PLAYER_STATE_DEFAULT,
  PLAYER_STATE_FREEFALL,
  PLAYER_STATE_ONGROUND,
  PLAYER_STATE_HEADCONTACT,
#ifdef GAME_JETPACK
  PLAYER_STATE_JETPACK_THRUST,
  PLAYER_STATE_JETPACK_FALL_IN_COLLISION,
  PLAYER_STATE_JETPACK_FALL
#endif
} player_state_t;
  
typedef struct {
  sprite_t sprite;
#ifdef PLAYER_HARDWARE_SPRITE
  sprite_hsprite_t* hsprite;
#endif
  int16_t animId;
  velocity_t velocity;
  sprite_animation_t* anim;
  sprite_save_t saves[2];
  int16_t flashCounter;
  int16_t frameCounter;
  int16_t freeFall;
  player_state_t state;
#ifdef GAME_JETPACK
  uint16_t jetpackFuel;
  uint16_t jetpackFallVelocity;
#endif
} player_t;

extern player_t player;

void 
player_init(menu_command_t command);
void 
player_saveBackground(frame_buffer_t fb);
void
player_restoreBackground(void);
void
player_render(frame_buffer_t fb);
void 
player_update(void);
void
player_setAction(int16_t action);
void
player_freeFall(void);
void
player_hSpriteRender(void);
void
player_updateCopper(void);


#endif
