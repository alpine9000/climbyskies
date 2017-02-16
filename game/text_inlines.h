#include "game.h"

#define FONTMAP_WIDTH_BYTES 32
#define FONTMAP_BIT_DEPTH   1

extern char* fontAtlas[128];

INLINE void
text_drawCharScoreBoard(char c, int x)
{
  char* src = fontAtlas[(int)c];
  char* dest = (char*)game_scoreBoardFrameBuffer+(x>>3)+(FRAME_BUFFER_WIDTH_BYTES*5);

  *dest = *src;
  *(dest+(1*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(1*FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH));
  *(dest+(2*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(2*FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH));
  *(dest+(3*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(3*FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH));
  *(dest+(4*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(4*FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH));
  *(dest+(5*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(5*FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH));
  *(dest+(6*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(6*FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH));
  *(dest+(7*FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)) = *(src+(7*FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH));
}

INLINE void
_text_drawChar8(frame_buffer_t frameBuffer, char c, int x, int y)
{
  USE(y);
  char* src = fontAtlas[(int)c];
  char* dest = (char*)frameBuffer+(x>>3);
  dest += (FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)*y;

  for (y = 0; y < 8; y++) {
    *dest = *src;
    dest += (FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH);
    src += FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH;
  }  
}

INLINE void
__text_drawChar8(frame_buffer_t frameBuffer, char c, int x, int y, int sy, int ny)
{
  USE(y);
  char* src = fontAtlas[(int)c];
  char* dest = (char*)frameBuffer+(x>>3);
  dest += (FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH)*y;

  src += (FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH)*sy;

  for (y = sy; y < ny; y++) {
    *dest = *src;
    dest += (FRAME_BUFFER_WIDTH_BYTES*SCREEN_BIT_DEPTH);
    src += FONTMAP_WIDTH_BYTES*FONTMAP_BIT_DEPTH;
  }  
}

INLINE void
text_drawChar8(frame_buffer_t fb, char c, int x, int y)
{
  //  int by = 0;
  int h = 8;
  y = y-game_screenScrollY;
  if (y >= 0) {
    //    (*render)(fb, image->x, by, sprite->x, y, image->w, h);
    __text_drawChar8(fb, c, x, y, 0, 8);
  } else {
    if (y > -h) {
      __text_drawChar8(fb, c, x, 0, -y, 8+y);
      __text_drawChar8(fb, c, x, FRAME_BUFFER_HEIGHT+y, 0, -y);
      //      (*render)(fb, image->x, by-y, sprite->x, 0, image->w, h+y);    
      //      (*render)(fb, image->x, by, sprite->x, FRAME_BUFFER_HEIGHT+y, image->w, -y);    
    } else {
      //      (*render)(fb, image->x, by, sprite->x, FRAME_BUFFER_HEIGHT+y, image->w, h);    
      __text_drawChar8(fb, c, x, FRAME_BUFFER_HEIGHT+y, 0, 8);
    }
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


