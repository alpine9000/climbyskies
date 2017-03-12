#include "game.h"

#ifdef DEBUG
void
_panic(char* message)
{
  message_screenOff();
  message_screenOn(message);
  for(;;);
}
#endif
