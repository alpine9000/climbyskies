#ifndef __TEXT_H
#define __TEXT_H

void 
text_drawText8Blitter(__REG("a0", frame_buffer_t frameBuffer), __REG("a1", char* string), __REG("d0", int32_t x), __REG("d1", int32_t y));
void 
text_drawMaskedText8Blitter(__REG("a0", frame_buffer_t frameBuffer), __REG("a1", char* string), __REG("d0", int32_t x), __REG("d1", int32_t y));
char*
text_intToAscii(__REG("d0", uint32_t number), __REG("d2", uint32_t numChars));
#ifdef INLINE_EVERYTHING
#include "text_inlines.h"
#else
void
text_drawScoreBoard(char* string, int32_t x);
void
text_drawCharScoreBoard(char c, int16_t x);
void
text_drawText8(frame_buffer_t frameBuffer, char* string, int32_t x, int32_t y);
#endif

#endif
