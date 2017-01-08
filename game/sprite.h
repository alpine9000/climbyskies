#ifndef __SPRITE_H
#define __SPRITE_H

extern uint16_t spriteBackground0;
extern uint16_t spriteBackground1;
extern uint16_t spriteBackground2;
extern uint16_t spriteBackground3;
extern uint16_t spriteBackground4;
extern uint16_t spriteBackground5;
extern uint16_t nullSprite;

extern void sprite_init(void);
extern void sprite_scroll(int scroll);
#endif
