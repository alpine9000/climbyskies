#include "game.h"

#ifdef DEBUG
__EXTERNAL uint16_t script_port = 0;

uint32_t script_breakpoint = 0xffffffff;

void
script_process(void)
{

  if (script_port > 0 && script_port <= 255) {
    keyboard_key = script_port;
  } else if (script_port & 0x8000) {
    script_breakpoint = script_port & 0x7fff;    
  }

  script_port = 0;
}
#endif
