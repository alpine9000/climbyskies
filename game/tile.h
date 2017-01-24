#ifndef __TILE_H
#define __TILE_H

extern uint16_t backgroundTiles[MAP_TILE_HEIGHT][MAP_TILE_WIDTH];

#define BACKGROUND_TILE(x,y) (*(&backgroundTiles[0][0] + ((y<<4) + (x>>4))))

void
tile_init(void);
void 
tile_renderScreen(void);
void
tile_renderNextTile(uint16_t hscroll);
void
tile_renderNextTileDown(uint16_t hscroll);
void
tile_invalidateTile(int x, int y, int offset);
void
tile_renderInvalidTiles(frame_buffer_t fb);

typedef struct tile_redraw {
  struct tile_redraw* prev;
  struct tile_redraw* next;
  int x;
  int y;
  int offset;
  int count;
} tile_redraw_t;
  

#endif
