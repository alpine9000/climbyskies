#ifndef __COPPER_H
#define __COPPER_H

#define AMIGA_SPRITES 8
#define NUM_SPRITE_COLUMNS 3
#define NUM_SPRITES 4

typedef struct {
  uint16_t bpl1[SCREEN_BIT_DEPTH*2*2];
#ifdef  PLAYER_HARDWARE_SPRITE
  uint16_t sprpt[8*4];
#endif
  uint16_t wait1[2];
  uint16_t wait2[2];
  uint16_t bpl2[SCREEN_BIT_DEPTH*2*2];
  uint16_t wait3[4];
  uint16_t bpl3[SCREEN_BIT_DEPTH*2*2];  
  uint16_t end[2];
} copper_t;

extern copper_t copper;

#endif
