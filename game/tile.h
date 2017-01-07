#ifndef __TILE_H
#define __TILE_H

void 
tile_renderScreen(frame_buffer_t frameBuffer);

uint32_t
tile_renderNextTile(frame_buffer_t frameBuffer, uint16_t hscroll);

#endif
