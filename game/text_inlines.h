#include "game.h"

#define FONTMAP_WIDTH_BYTES 32

extern char* fontAtlas[128];

INLINE void
text_drawCharScoreBoard(char c, int x)
{
  char* src = fontAtlas[(int)c];
  char* dest = (char*)game_scoreBoardFrameBuffer+(x>>3);

  *dest = *src;
  *(dest+(1*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(1*FONTMAP_WIDTH_BYTES));
  *(dest+(2*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(2*FONTMAP_WIDTH_BYTES));
  *(dest+(3*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(3*FONTMAP_WIDTH_BYTES));
  *(dest+(4*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(4*FONTMAP_WIDTH_BYTES));
  *(dest+(5*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(5*FONTMAP_WIDTH_BYTES));
  *(dest+(6*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(6*FONTMAP_WIDTH_BYTES));
  *(dest+(7*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(7*FONTMAP_WIDTH_BYTES));
}

INLINE void
text_drawChar8(frame_buffer_t frameBuffer, char c, int x, int y)
{
  USE(y);
  char* src = fontAtlas[(int)c];
  char* dest = (char*)frameBuffer+(x>>3);

  for (y = 0; y < 8; y++) {
    *dest = *src;
    dest += (FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH);
    src += FONTMAP_WIDTH_BYTES;
  }  
}

INLINE void
text_drawText8(frame_buffer_t frameBuffer, char* string, int32_t x, int32_t y)
{
  char* ptr = &string[0]; 
  do {
    text_drawChar8(frameBuffer, *ptr, x, y);
    ptr++;
    x += 8;
  } while (*ptr != 0);
}

INLINE void
text_drawScoreBoard(char* string, int32_t x)
{
  char* ptr = &string[0]; 
  do {
    text_drawCharScoreBoard(*ptr, x);
    ptr++;
    x += 8;
  } while (*ptr != 0);
}
