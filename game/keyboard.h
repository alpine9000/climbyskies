#ifndef __KEYBOARD_H
#define __KEYBOARD_H

extern uint16_t keyboard_code;
void
keyboard_ctor(void);
void
keyboard_dtor(void);

char 
keyboard_getKey(void);

#endif
