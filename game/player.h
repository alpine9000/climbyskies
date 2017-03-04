#ifndef __PLAYER_H
#define __PLAYER_H

#define PLAYER_WIDTH                32
#define PLAYER_FUZZY_WIDTH          8
#define PLAYER_VISIBLE_WIDTH        (PLAYER_WIDTH-PLAYER_FUZZY_WIDTH)
#define PLAYER_HEIGHT               37
#define PLAYER_BASE_PLATFORM_HEIGHT (TILE_HEIGHT*3)

#ifdef PLAYER_RECORDING

#define PLAYER_MAX_RECORD 1024

typedef struct {
  uint8_t joystickPos;
  uint8_t joystickButton;
  uint16_t frame;
} player_record_item_t;

typedef enum {
  PLAYER_RECORD_IDLE,
  PLAYER_RECORD_RECORD,
  PLAYER_RECORD_PLAYBACK
} player_record_state_t;

typedef struct {
  uint32_t size;
  player_record_state_t state;
  uint32_t index;
  uint32_t lastJoystickPos;
  uint32_t lastJoystickButton;
  uint8_t joystickPos;
  uint8_t joystickButton;
  uint16_t frame;
  player_record_item_t buffer[PLAYER_MAX_RECORD];
} player_record_t;

#endif

typedef enum {
  PLAYER_STATE_DEFAULT,
  PLAYER_STATE_FREEFALL,
  PLAYER_STATE_ONGROUND,
  PLAYER_STATE_HEADCONTACT,
  PLAYER_STATE_JETPACK_THRUST,
  PLAYER_STATE_JETPACK_FALL
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
#ifdef PLAYER_BLIT_SPRITE_OVERDRAW
  uint16_t hspriteCompatible;
#endif
  uint16_t jetpackFuel;
  uint16_t jetpackFallVelocity;
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

#ifdef PLAYER_RECORDING
void
player_setRecord(player_record_state_t state);
player_record_state_t
player_getRecord(void);
#endif

#endif
