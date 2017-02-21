#ifndef ITEM_H
#define ITEM_H

#define ITEM_HEIGHT 11
#define ITEM_WIDTH 16


typedef enum {
  ITEM_ANIM_COIN = 0,
  ITEM_ANIM_LIGHTNING, // Ability to kill enemies
  ITEM_ANIM_HEART, // Extra life
  ITEM_ANIM_HEADSMASH, // Smash non smashable platforms
} item_anim_t;

extern int16_t item_count;

void
item_init(void);
void
item_saveBackground(frame_buffer_t fb);
void
item_restoreBackground(void);
void
item_render(frame_buffer_t fb);
void
item_update(sprite_t* p);
void
item_add(int16_t x, int16_t y, int16_t anim, uint16_t* tilePtr);
#endif
