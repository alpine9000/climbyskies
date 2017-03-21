#include "game.h"

#define MAX_INVALID_TILES 24

uint16_t* tile_tilePtr;
uint16_t* tile_itemPtr;
int16_t tile_tileX;
#ifdef DEBUG
uint16_t tile_invalidTileCount = 0;
#endif
static __section(random_c) tile_redraw_t* invalidTiles;
static __section(random_c) tile_redraw_t* invalidFreeList;
static __section(random_c) tile_redraw_t invalidTileBuffers[MAX_INVALID_TILES];

void
tile_init(void)
{
#ifdef DEBUG
  tile_invalidTileCount = 0;
#endif
  invalidTiles = 0;
  invalidFreeList = &invalidTileBuffers[0];
   invalidFreeList->prev = 0;
  //  invalidFreeList->next = 0;
  tile_redraw_t* ptr = invalidFreeList;
  for (int16_t i = 1; i < MAX_INVALID_TILES; i++) {
      ptr->next = &invalidTileBuffers[i];
      ptr->next->prev = ptr;
      ptr = ptr->next;
  }

  ptr->next = 0;
}


static tile_redraw_t*
tile_getFree(void)
{
  tile_redraw_t* entry = invalidFreeList;
  invalidFreeList = invalidFreeList->next;
  invalidFreeList->prev = 0;
  return entry;
}


static void
tile_addFree(tile_redraw_t* ptr)
{
#ifdef DEBUG
  tile_invalidTileCount--;
#endif
  if (invalidFreeList == 0) {
    invalidFreeList = ptr;
    ptr->next = 0;
    ptr->prev = 0;
  } else {
    ptr->next = invalidFreeList;
    ptr->next->prev = ptr;
    ptr->prev = 0;
    invalidFreeList = ptr;
  }
}


static void
tile_addInvalid(tile_redraw_t* ptr)
{
#ifdef DEBUG
  tile_invalidTileCount++;
  if (tile_invalidTileCount >= MAX_INVALID_TILES) {
    PANIC("tile_addInvalid: no free links");
  }
#endif

  if (invalidTiles == 0) {
    invalidTiles = ptr;
    ptr->next = 0;
    ptr->prev = 0;
  } else {
    ptr->next = invalidTiles;
    ptr->next->prev = ptr;
    ptr->prev = 0;
    invalidTiles = ptr;
  }
}


static  void
tile_removeInvalid(tile_redraw_t* ptr)
{
  if (ptr->prev == 0) {
    invalidTiles = ptr->next;
    if (invalidTiles) {
      invalidTiles->prev = 0;
    }
  } else {
    ptr->prev->next = ptr->next;
    if (ptr->next != 0) {
      ptr->next->prev = ptr->prev;
    }
  }
}


void
tile_invalidateTile(int16_t x, int16_t y, uint16_t offset)
{
  tile_redraw_t* ptr = tile_getFree();
  ptr->x = x;
  ptr->y = y;
  ptr->offset = offset;
  ptr->count = 2;
  tile_addInvalid(ptr);
}


void
tile_renderInvalidTiles(frame_buffer_t fb)
{
  tile_redraw_t* ptr = invalidTiles;

  while (ptr != 0) {
    gfx_renderTile(fb, ptr->x, ptr->y, level.spriteBitplanes+ptr->offset);
    ptr->count--;
    tile_redraw_t* save = ptr;
    ptr = ptr->next;
    if (save->count == 0) {
      tile_removeInvalid(save);
      tile_addFree(save);
    }    
  }
}


void 
tile_renderScreen(void)
{
  tile_tileX = SCREEN_WIDTH-TILE_WIDTH;

  tile_tilePtr = &level.tileAddresses[MAP_TILE_HEIGHT-1][MAP_TILE_WIDTH-1];
  tile_itemPtr = &level.item_spriteIds[MAP_TILE_HEIGHT-1][MAP_TILE_WIDTH-1];
  for (int16_t y = SCREEN_HEIGHT-TILE_HEIGHT; y >= 0; y-=TILE_HEIGHT) {
    for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
      uint16_t offset = *tile_tilePtr; // todo: 16 ?
      gfx_renderTileOffScreen(game_offScreenBuffer, x, y, level.spriteBitplanes+offset);
      gfx_renderTileOffScreen(game_onScreenBuffer, x, y, level.spriteBitplanes+offset);
      uint16_t item = *tile_itemPtr;
      if (item != 0) {
	tile_processMapObjectDown(item, x, WORLD_HEIGHT-SCREEN_HEIGHT+y, tile_itemPtr);
      }
      tile_tilePtr--;
      tile_itemPtr--;
    }
  }
  
  int16_t y = FRAME_BUFFER_MAX_HEIGHT-TILE_HEIGHT;
  for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
    uint16_t offset = *tile_tilePtr; // todo: 16  ?
    gfx_renderTileOffScreen(game_onScreenBuffer, x, y, level.spriteBitplanes+offset);
    gfx_renderTileOffScreen(game_offScreenBuffer, x, y, level.spriteBitplanes+offset);
    if (*tile_itemPtr != 0) {
      tile_processMapObjectDown(*tile_itemPtr, x, WORLD_HEIGHT-SCREEN_HEIGHT+y, tile_itemPtr);
    }
    tile_tilePtr--;
    tile_itemPtr--;
  }
}


#ifndef INLINE_EVERYTHING
#include "tile_inlines.h"
#endif
