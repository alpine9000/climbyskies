INLINE void
tile_processMapObjectUp(uint16_t id, int16_t x, int16_t y, uint16_t* tilePtr)
{
  if (id & MAPOBJECT_ITEM_FLAG) {
    item_add(x, y, id & 0xFF, tilePtr);
  } else if (id & MAPOBJECT_TOP_RENDER_ENEMY_FLAG) {
    uint16_t* tilePtrHi = tilePtr;
    do {
      tilePtr += MAP_TILE_WIDTH;
    } while (!(*tilePtr & MAPOBJECT_BOTTOM_RENDER_ENEMY_FLAG));    
    enemy_addMapObject(id & 0xff, x, y+(3*TILE_HEIGHT), tilePtrHi, tilePtr);
  }
}


INLINE void
tile_processMapObjectDown(uint16_t id, int16_t x, int16_t y, uint16_t* tilePtr)
{
  if (id & MAPOBJECT_ITEM_FLAG) {
    item_add(x, y, id & 0xFF, tilePtr);
  } else if (id & MAPOBJECT_BOTTOM_RENDER_ENEMY_FLAG) {
    uint16_t* tilePtrLo = tilePtr;
    do {
      tilePtr -= MAP_TILE_WIDTH;
    } while (!(*tilePtr & MAPOBJECT_TOP_RENDER_ENEMY_FLAG));    
    enemy_addMapObject(id & 0xff, x, y+TILE_HEIGHT, tilePtr, tilePtrLo);
  }
}


INLINE void
tile_renderNextTile(uint16_t hscroll, uint16_t itemY)
{
  int16_t y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

  uint16_t offset = *tile_tilePtr;

  gfx_quickRenderTileOffScreen(game_offScreenBuffer, tile_tileX, y, level.spriteBitplanes+offset);
  gfx_quickRenderTileOffScreen(game_onScreenBuffer, tile_tileX, y, level.spriteBitplanes+offset);

  if (*tile_itemPtr != 0) {
    if (tile_itemPtr > &level.item_spriteIds[0][0]) {
      tile_processMapObjectDown(*tile_itemPtr, tile_tileX, (itemY-TILE_HEIGHT) & 0xfff0, tile_itemPtr);
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
tile_renderNextTileDown(uint16_t hscroll, uint16_t itemY)
{
  int16_t y = (FRAME_BUFFER_HEIGHT-hscroll-(3*TILE_HEIGHT));

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

#define _OFFSET (((FRAME_BUFFER_HEIGHT-(1*TILE_HEIGHT))/TILE_HEIGHT)*(SCREEN_WIDTH/TILE_WIDTH))
  uint16_t offset = *(tile_tilePtr+_OFFSET);
  gfx_quickRenderTileOffScreen(game_offScreenBuffer, tile_tileX, y, level.spriteBitplanes+offset);
  gfx_quickRenderTileOffScreen(game_onScreenBuffer, tile_tileX, y, level.spriteBitplanes+offset);

  uint16_t* ptr = tile_itemPtr+_OFFSET;

  if (*(ptr) != 0) {
    if (ptr < &level.item_spriteIds[MAP_TILE_HEIGHT-1][MAP_TILE_WIDTH-1]) {    
      tile_processMapObjectUp(*(ptr), tile_tileX, (itemY+SCREEN_HEIGHT+(TILE_HEIGHT*2))& 0xfff0, ptr);
    }
  }  

  tile_itemPtr = tile_itemPtr+1;
  tile_tilePtr = tile_tilePtr+1;
  tile_tileX += TILE_WIDTH;

  if (tile_tileX > SCREEN_WIDTH-TILE_WIDTH) {
    tile_tileX = 0;
  }
}
