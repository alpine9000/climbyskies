#ifndef __GFX_H
#define __GFX_H

#include <hardware/blit.h>

#define gfx_retroFontWidth 5  
#define gfx_retroFontHeight 8

extern frame_buffer_t spriteFrameBuffer;
extern frame_buffer_t spriteMask;
extern const unsigned char font[];

void 
gfx_init(void);
void
gfx_fillRect(frame_buffer_t fb, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void
gfx_renderSprite(frame_buffer_t dest, int16_t sx, int16_t sy, int16_t dx, int16_t dy, int16_t w, int16_t h);
void
gfx_saveSprite(frame_buffer_t source, int16_t dx, int16_t dy, int16_t w, int16_t h);
void
gfx_clearSprite(frame_buffer_t dest, int16_t dx, int16_t dy, int16_t w, int16_t h);
void
gfx_renderTile(frame_buffer_t dest, int16_t sx, int16_t sy, int16_t dx, int16_t dy);

void
gfx_renderTile2(frame_buffer_t dest, int16_t sx, int16_t sy, frame_buffer_t tile);
#endif
