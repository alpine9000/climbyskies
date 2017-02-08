INLINE void
tile_renderNextTile(uint16_t hscroll)
{
  int y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

  unsigned long offset = *tile_tilePtr;
  gfx_renderTileOffScreen(offScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);
  gfx_renderTileOffScreen(onScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);
  
  tile_tilePtr = tile_tilePtr-1;

  tile_tileX -= TILE_WIDTH;

  if (tile_tileX < 0) {
    tile_tileX = SCREEN_WIDTH-TILE_WIDTH;
  }
}


INLINE void
tile_renderNextTileDown(uint16_t hscroll)
{
  int y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

  tile_tilePtr = tile_tilePtr+1;
  tile_tileX += TILE_WIDTH;

  if (tile_tileX > SCREEN_WIDTH-TILE_WIDTH) {
    tile_tileX = 0;
  }

#define OFFSET (((FRAME_BUFFER_HEIGHT-(1*TILE_HEIGHT))/TILE_HEIGHT)*(SCREEN_WIDTH/TILE_WIDTH))
  unsigned long offset = *(tile_tilePtr+OFFSET);
  gfx_renderTileOffScreen(offScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);
  gfx_renderTileOffScreen(onScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);
}
