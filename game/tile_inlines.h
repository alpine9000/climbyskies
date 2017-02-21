INLINE void
tile_processMapObjectUp(uint16_t id, int16_t x, int16_t y, uint16_t* tilePtr)
{
  if (id & MAPOBJECT_ITEM_FLAG) {
    item_add(x, y, id & 0xFF, tilePtr);
  } else if (id & MAPOBJECT_TOP_RENDER_ENEMY_FLAG) {
    enemy_addMapObject(id & 0xff, x, y+(3*TILE_HEIGHT), tilePtr);
  }
}

INLINE void
tile_processMapObjectDown(uint16_t id, int16_t x, int16_t y, uint16_t* tilePtr)
{
  if (id & MAPOBJECT_ITEM_FLAG) {
    item_add(x, y, id & 0xFF, tilePtr);
  } else if (id & MAPOBJECT_BOTTOM_RENDER_ENEMY_FLAG) {
    enemy_addMapObject(id & 0xff, x, y+TILE_HEIGHT, tilePtr);
  }
}

INLINE void
tile_renderNextTile(uint16_t hscroll)
{
  int16_t y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

  uint16_t offset = *tile_tilePtr; // todo: 16 ?

  gfx_quickRenderTileOffScreen(game_offScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);
  gfx_quickRenderTileOffScreen(game_onScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);

  if (*tile_itemPtr != 0) {
    if (tile_itemPtr > &level.item_spriteIds[0][0]) {
      //      item_addCoin(tile_tileX, ((game_cameraY>>4)<<4)-16, tile_itemPtr);
      tile_processMapObjectDown(*tile_itemPtr, tile_tileX, ((game_cameraY>>4)<<4)-16, tile_itemPtr);
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
  int16_t y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));

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
  uint16_t offset = *(tile_tilePtr+OFFSET); // todo: 16 ?
  gfx_quickRenderTileOffScreen(game_offScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);
  gfx_quickRenderTileOffScreen(game_onScreenBuffer, tile_tileX, y, spriteFrameBuffer+offset);

  int16_t itemOffset = 16;
  uint16_t* ptr = tile_itemPtr+OFFSET;

  if (*(ptr) != 0) {
    if (ptr < &level.item_spriteIds[MAP_TILE_HEIGHT-1][MAP_TILE_WIDTH-1]) {
      //      item_addCoin(tile_tileX, (((game_cameraY+8)>>4)<<4)+SCREEN_HEIGHT+itemOffset, ptr);
      //      tile_processMapObjectUp(*(ptr), tile_tileX, (((game_cameraY+8)>>4)<<4)+SCREEN_HEIGHT+itemOffset, ptr);
      tile_processMapObjectUp(*(ptr), tile_tileX, (((game_cameraY+8)>>4)<<4)+SCREEN_HEIGHT+itemOffset, ptr);
    }
  }  
}
