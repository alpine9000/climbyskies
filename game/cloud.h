#ifndef __CLOUD_H
#define __CLOUD_H

void 
cloud_init(frame_buffer_t fb);
void 
cloud_saveBackground(frame_buffer_t fb);
void 
cloud_restoreBackground(void);
void 
cloud_render(frame_buffer_t fb);
void 
cloud_update(void);

#endif
