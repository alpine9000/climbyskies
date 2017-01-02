#ifndef __GFX_H
#define __GFX_H

#include <hardware/blit.h>

#define gfx_retroFontWidth 5  
#define gfx_retroFontHeight 8

extern const unsigned char font[];

void
gfx_fillRect(frame_buffer_t fb, uint16 x, uint16 y, uint16 w, uint16 h, uint16 color);
void 
gfx_drawStringRetro(frame_buffer_t fb, int16 x, int16 y, char *c, uint16 color, int spaceSize);
void
gfx_drawCharRetro(frame_buffer_t fb, int16 x, int16 y, char c, uint16 color);
void 
gfx_drawLine(frame_buffer_t fb, int16 x0, int16 y0, int16 x1, int16 y1, uint16 color);
uint8
gfx_getPixel(frame_buffer_t fb, int16 x, int16 y);
void
gfx_drawPixel(frame_buffer_t fb, int16 x, int16 y, uint16 color);
void
gfx_bitBlt(frame_buffer_t dest, int16 sx, int16 sy, int16 dx, int16 dy, int16 w, int16 h, frame_buffer_t source);
void 
gfx_init(void);

#endif
