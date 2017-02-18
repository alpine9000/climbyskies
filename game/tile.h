#ifndef __TILE_H
#define __TILE_H

extern unsigned short* tile_tilePtr;
extern unsigned short* tile_itemPtr;
extern int16_t tile_tileX;

#define TILE_SKY 0xbba6
#define TILE_COLLISION(x) (x < 0x1900)
#define BACKGROUND_TILE(x,y) (*(&level.background_tileAddresses[0][0] + ((((y)>>4)<<4) + ((x)>>4))))
//#define BACKGROUND_TILE(x,y) (*(&backgroundTiles[0][0] + ((((y/TILE_HEIGHT)*TILE_WIDTH)) + (x/TILE_WIDTH))))

void
tile_init(void);
void 
tile_renderScreen(void);
void
tile_invalidateTile(int16_t x, int16_t y, int16_t offset);
void
tile_renderInvalidTiles(frame_buffer_t fb);

#ifndef INLINE_EVERYTHING
void
tile_renderNextTile(uint16_t hscroll);
void
tile_renderNextTileDown(uint16_t hscroll);
#else
#include "tile_inlines.h"
#endif


typedef struct tile_redraw {
  struct tile_redraw* prev;
  struct tile_redraw* next;
  int16_t x;
  int16_t y;
  int16_t offset;
  int16_t count;
} tile_redraw_t;
  

#endif
