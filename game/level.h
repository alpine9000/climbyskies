#ifndef LEVEL_H
#define LEVEL_H

typedef struct {
  unsigned short background_tileAddresses[202][16];
  unsigned short item_tileAddresses[202][16];
} level_t;

extern level_t level1;
extern level_t level;

#endif
