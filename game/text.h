#ifndef __TEXT_H
#define __TEXT_H

void 
text_drawText8(__reg("a0") frame_buffer_t frameBuffer, __reg("a1") char* string, __reg("d0") int32_t x, __reg("d1") int32_t y);
char*
text_intToAscii(__reg("d0") uint32_t number, __reg("d2") uint32_t numChars);
void
text_draw(frame_buffer_t fb, char* str, int x, int y);

#endif
