#include "game.h"

extern uint32_t keyboard_code;
extern char keyboard_keymap[];

static char keyboard_lastKey = 0;

char keyboard_getKey(void)
{
  char c = keyboard_keymap[keyboard_code>>16];
  if (c != keyboard_lastKey) {
    keyboard_lastKey = c;
    return c;
  }
  keyboard_lastKey = c;
  return 0;
}
