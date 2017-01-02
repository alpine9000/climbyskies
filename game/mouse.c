#include "game.h"

uint32_t
mouse_leftButtonPressed(void)
{
  uint8_t* ciaa = (uint8_t*)0xbfe001;
  return !(*ciaa & 0x40);
}
