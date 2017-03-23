#include "game.h"

__EXTERNAL uint16_t script_port = 0;

uint32_t script_breakpoint = 0xffffffff;

void
script_process(void)
{

  switch (script_port) {
  case 0:
    break;
  case 1:
    script_breakpoint = 1000;
    break;
  default: // keys
    keyboard_key = script_port;
    break;
  }  

  script_port = 0;
}
