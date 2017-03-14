#ifndef __MENU_H
#define __MENU_H

typedef enum {
  MENU_COMMAND_PLAY = 0,
  MENU_COMMAND_EXIT,
  MENU_COMMAND_REPLAY,
  MENU_COMMAND_RECORD,
  MENU_COMMAND_LEVEL
} menu_command_t;

typedef enum {
  MENU_MODE_MENU,
  MENU_MODE_HISCORES
} menu_mode_t;

menu_command_t
menu_loop(menu_mode_t mode);

#endif
