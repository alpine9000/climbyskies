#ifndef ENEMY_H
#define ENEMY_H

void
enemy_add(int x, int y, int anim);
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

#endif
