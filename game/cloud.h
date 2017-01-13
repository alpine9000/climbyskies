#ifndef __CLOUD_H
#define __CLOUD_H

extern void cloud_init(frame_buffer_t fb);
extern void cloud_saveBackground(frame_buffer_t fb);
extern void cloud_restoreBackground(frame_buffer_t fb);
extern void cloud_render(frame_buffer_t fb);
extern void cloud_update(void);

#endif
