#ifndef __HISCORE_H
#define __HISCORE_H


#define HISCORE_NUM_SCORES (MENU_NUM_ITEMS-1)

typedef struct {
  uint32_t score;
  char name[4];
  uint32_t checksum;
  char text[16];
} hiscore_t;

void
hiscore_ctor(void);

uint16_t
hiscore_load(void);

hiscore_t*
hiscore_render(void);

void
hiscore_addScore(uint32_t score);

void
hiscore_default(void);
#endif
