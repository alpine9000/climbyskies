#include "game.h"

void
palette_black()
{
  for (int16_t i = 0; i < 32; i++) {
    custom->color[i] = 0x000;
  }
}

extern uint16_t fadeInFadeTable[64*32];
void
palette_fadeIn(void)
{
  for (int16_t i = 0; i < 64; i++) {
    for (int16_t c = 0; c < 32; c++) {
      custom->color[c] = fadeInFadeTable[(i*32)+c];
    }

    hw_waitScanLines(100);
  }
}


extern uint16_t menuFadeInFadeTable[64*32];
void
palette_menuFadeIn(void)
{
  for (int16_t i = 0; i < 64; i++) {
    for (int16_t c = 0; c < 32; c++) {
      custom->color[c] = menuFadeInFadeTable[(i*32)+c];
    }

    hw_waitScanLines(100);
  }
}
