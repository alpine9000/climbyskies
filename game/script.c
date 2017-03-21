#include "game.h"

__EXTERNAL uint16_t script_port = 0;

void
script_process(void)
{

  switch (script_port) {
  case 1:
    keyboard_key = 'P';
    break;
  case 2:
    keyboard_key = '2';
    break;
  }  

  script_port = 0;
}
