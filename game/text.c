#include "game.h"

#ifndef INLINE_EVERYTHING
#include "text_inlines.h"
#endif


static char _text_hex[] = {'A', 'B', 'C', 'D', 'E', 'F'};
static char _text_buf[100];

static inline 
int _text_hexChar(int s)
{
  int c;

  c = (s >= 0 && s <= 9) ? s + '0' : _text_hex[s - 10];
  return c;
}  


char*
text_intToHex(uint32_t n, uint16_t numChars)
{
  uint32_t c;
  char* ptr = &_text_buf[numChars];
  *ptr = 0;
  ptr--;
  for (c = 1; c <= numChars; c++) {
    *ptr = _text_hexChar(n & 0xf);
    ptr--;
    n = n >> 4;
  }

  return _text_buf;
}


