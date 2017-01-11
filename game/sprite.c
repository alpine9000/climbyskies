#include "game.h"

#define NUM_SCROLLED_SPRITES 4

typedef struct {
  uint16_t control[2];
  uint16_t data[2];
  uint16_t* ptr;
} sprite_t;
  

static sprite_t sprites[NUM_SCROLLED_SPRITES] = {
  {
    {0x1d48, 0xff02},
    {0,0},
    &spriteBackground0
  },
  {
    {0x1d50, 0xff02},
    {0,0},
    &spriteBackground1
  },
  {
    {0x1d58, 0xff02},
    {0,0},
    &spriteBackground2
  },
  {
    {0x1d60, 0xff02},
    {0,0},
    &spriteBackground3
  }
};


void
sprite_init(void)
{
  sprite_scroll(SPRITE_BACKGROUND_HEIGHT-16);

  copper.sprites[0x11] = (uint32_t)&spriteBackground4 >> 16;
  copper.sprites[0x13] = (uint32_t)&spriteBackground4;
  copper.sprites[0x15] = (uint32_t)&spriteBackground5 >> 16;
  copper.sprites[0x17] = (uint32_t)&spriteBackground5;
  copper.sprites[0x19] = (uint32_t)&nullSprite >> 16;
  copper.sprites[0x1b] = (uint32_t)&nullSprite;
  copper.sprites[0x1d] = (uint32_t)&nullSprite >> 16;
  copper.sprites[0x1f] = (uint32_t)&nullSprite;

  custom->dmacon = (DMAF_SPRITE|DMAF_SETCLR);
}

void
sprite_scroll(int scroll)
{
  for (int i = 0; i < NUM_SCROLLED_SPRITES; i++) {
    uint16_t* newPtr = sprites[i].ptr+scroll;
    sprites[i].data[1] = newPtr[1];
    sprites[i].data[0] = newPtr[0];
    newPtr[1] = sprites[i].control[1];
    newPtr[0] = sprites[i].control[0];
    copper.sprites[(i*4)+1] = (uint32_t)(newPtr) >> 16;
    copper.sprites[(i*4)+3] = (uint32_t)(newPtr);
    sprites[i].ptr[1] = sprites[i].data[1];
    sprites[i].ptr[0] = sprites[i].data[0];
    sprites[i].ptr = newPtr;
  }
}
