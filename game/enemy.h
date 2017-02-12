#ifndef ENEMY_H
#define ENEMY_H

#define ENEMY_RUN_HEIGHT 35

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
#endif
