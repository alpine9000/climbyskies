#include "game.h"

__EXTERNAL uint16_t script_port = 0;

void
script_process(void)
{

  switch (script_port) {
  case 0:
    break;
  default: // keys
    keyboard_key = script_port;
    break;
  }  

  script_port = 0;
}
