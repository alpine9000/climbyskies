INLINE void
tile_renderNextTile(uint16_t hscroll)
{
  int y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

  unsigned long offset = *tile_tilePtr;

  gfx_quickRenderTileOffScreen(game_offScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);
  gfx_quickRenderTileOffScreen(game_onScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);

  if (*tile_itemPtr != 0) {
    if (tile_itemPtr > &level.item_tileAddresses[0][0]) {
      item_addCoin(tile_tileX, ((game_cameraY>>4)<<4)-16);
    }
  }  

  tile_tilePtr = tile_tilePtr-1;
  tile_itemPtr = tile_itemPtr-1;

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

  tile_itemPtr = tile_itemPtr+1;
  tile_tilePtr = tile_tilePtr+1;
  tile_tileX += TILE_WIDTH;


  if (tile_tileX > SCREEN_WIDTH-TILE_WIDTH) {
    tile_tileX = 0;
  }

#define OFFSET (((FRAME_BUFFER_HEIGHT-(1*TILE_HEIGHT))/TILE_HEIGHT)*(SCREEN_WIDTH/TILE_WIDTH))
  unsigned long offset = *(tile_tilePtr+OFFSET);
  gfx_quickRenderTileOffScreen(game_offScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);
  gfx_quickRenderTileOffScreen(game_onScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);

  int itemOffset = 16;
  unsigned short* ptr = tile_itemPtr+OFFSET;

  if (*(ptr) != 0) {
    if (ptr < &level.item_tileAddresses[MAP_TILE_HEIGHT-1][MAP_TILE_WIDTH-1]) {
      item_addCoin(tile_tileX, (((game_cameraY+8)>>4)<<4)+SCREEN_HEIGHT+itemOffset);
    }
  }  
}
