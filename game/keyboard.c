#include "game.h"

extern uint16_t keyboard_code;
extern char keyboard_keymap[];

static uint16_t keyboard_lastCode = 0;

char 
keyboard_getKey(void)
{
  char c = 0;

  if (keyboard_code != keyboard_lastCode) {
    if (keyboard_code <= 0x47) {
      c = keyboard_keymap[keyboard_code];
    }
  }

  keyboard_lastCode = keyboard_code;

  return c;
}
