
#ifndef __CLOUD_H
#define __CLOUD_H

#define CLOUD_HEIGHT 27
#define CLOUD_WIDTH 16*3

void 
cloud_init(void);
void 
cloud_saveBackground(frame_buffer_t fb);
void 
cloud_restoreBackground(void);
void 
cloud_render(frame_buffer_t fb);
void 
cloud_update(void);

#endif
