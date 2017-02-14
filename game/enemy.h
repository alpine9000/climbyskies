#ifndef ENEMY_H
#define ENEMY_H

#define ENEMY_RUN_HEIGHT 35

typedef enum {
  ENEMY_ANIM_RIGHT_RUN = 0,
  ENEMY_ANIM_LEFT_RUN,
  ENEMY_ANIM_RIGHT_SKATE,
  ENEMY_ANIM_LEFT_SKATE,
  ENEMY_ANIM_RIGHT_DRAGON
} enemy_anim_t;

int enemy_count;

void
enemy_init(void);
void
enemy_saveBackground(frame_buffer_t fb);
void
enemy_restoreBackground(void);
void
enemy_render(frame_buffer_t fb);
void
enemy_update(sprite_t* p);
void
enemy_ctor(void);
void
enemy_addMapObject(int id, int x, int y, unsigned short* tilePtr);
#endif
