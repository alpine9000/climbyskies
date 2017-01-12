#ifndef __BOB_H
#define __BOB_H

extern void bob_render(frame_buffer_t fb, int16_t x, int16_t y, uint16_t b);
extern void bob_save(frame_buffer_t fb, int16_t x, int16_t y, uint16_t b);
extern void bob_clear(frame_buffer_t fb, int16_t x, int16_t y, uint16_t b);

#endif
