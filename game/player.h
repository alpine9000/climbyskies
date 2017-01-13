#ifndef __PLAYER_H
#define __PLAYER_H

extern void player_init(frame_buffer_t fb);

extern void player_saveBackground(frame_buffer_t fb);
extern void player_restoreBackground(frame_buffer_t fb);
extern void player_render(frame_buffer_t fb);
extern void player_update(void);

extern void player_setAction(int action);

#endif
