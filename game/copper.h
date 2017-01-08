#ifndef __COPPER_H
#define __COPPER_H

#define AMIGA_SPRITES 8
#define NUM_SPRITE_COLUMNS 3
#define NUM_SPRITES 4

typedef struct {
  uint16_t wait[2];
  uint16_t pos[NUM_SPRITES*2];
} sprite_pos_t;

typedef struct {
  uint16_t wait1[2];
  uint16_t wait2[2];
  uint16_t bpl[SCREEN_BIT_DEPTH*2*2];
  sprite_pos_t sprite[NUM_SPRITE_COLUMNS];
} copper_video_line_t;

typedef struct {
  uint16_t wait[2];
  uint16_t sprites[AMIGA_SPRITES*4];
  copper_video_line_t lines[SCREEN_HEIGHT];
  uint16_t end[2];
} copper_t;

extern copper_t copper;

#endif
