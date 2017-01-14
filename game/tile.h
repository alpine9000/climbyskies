#ifndef __TILE_H
#define __TILE_H

#define BACKGROUND_TILE(x,y) (*(&background_tileAddresses[0][0] + ((y<<4) + (x>>4))))

void 
tile_renderScreen(void);

uint32_t 
tile_renderNextTile(uint16_t hscroll);
#endif
