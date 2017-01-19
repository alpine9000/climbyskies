#ifndef __PLAYER_H
#define __PLAYER_H

void 
player_init(void);
void 
player_saveBackground(frame_buffer_t fb);
void
player_restoreBackground(void);
void
player_render(frame_buffer_t fb);
void 
player_update(void);
void
player_setAction(int action);

#endif
