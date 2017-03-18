#ifndef ENEMY_H
#define ENEMY_H

#define ENEMY_RUN_HEIGHT 35

typedef enum {
  ENEMY_ANIM_RIGHT_RUN = 0,
  ENEMY_ANIM_LEFT_RUN,
  ENEMY_ANIM_RIGHT_SKATE,
  ENEMY_ANIM_LEFT_SKATE,
  ENEMY_ANIM_RIGHT_DRAGON,
  ENEMY_ANIM_JOYSTICK
} enemy_anim_t;

void
enemy_init(void);
void
enemy_saveBackground(frame_buffer_t fb);
void
enemy_restoreBackground(void);
void
enemy_render(frame_buffer_t fb);
void
enemy_update(void);
void
enemy_ctor(void);
void
enemy_addMapObject(int16_t id, int16_t x, int16_t y, uint16_t* tilePtrHi, uint16_t* tilePtrLo);
int16_t
enemy_headsmash(int16_t x, int16_t y);
uint16_t
enemy_find(uint16_t id);
#ifdef DEBUG
int16_t enemy_getCount(void);
#endif
#endif
