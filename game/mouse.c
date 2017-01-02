#include "game.h"

int
mouse_leftButtonPressed(void)
{
  uint8* ciaa = (uint8*)0xbfe001;
  return !(*ciaa & 0x40);
}
