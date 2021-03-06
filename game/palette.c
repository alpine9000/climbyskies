#include "game.h"

uint16_t palette_background = 0;

void
palette_black()
{
  for (int16_t i = 0; i < 32; i++) {
    custom->color[i] = 0x000;
  }

  palette_background = 0;
}

void
palette_fadeIn(uint16_t* fadeInFadeTable)
{
  for (int16_t i = 0; i < 64; i++) {
    for (int16_t c = 0; c < 32; c++) {
      custom->color[c] = fadeInFadeTable[(i*32)+c];
    }

    hw_waitScanLines(100);
  }

  palette_background = fadeInFadeTable[63*32];
}

uint16_t
palette_getColor(uint16_t* fadeInFadeTable, uint16_t index)
{
  return fadeInFadeTable[(63*32)+index];
}
