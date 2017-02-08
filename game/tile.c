#include "game.h"

#define MAX_INVALID_TILES 100

unsigned short* tile_tilePtr;
int tile_tileX;
static tile_redraw_t* invalidTiles = 0;
static tile_redraw_t* invalidFreeList = 0;
static tile_redraw_t invalidTileBuffers[MAX_INVALID_TILES];

extern uint16_t background_tileAddresses[MAP_TILE_HEIGHT][MAP_TILE_WIDTH];

uint16_t backgroundTiles[MAP_TILE_HEIGHT][MAP_TILE_WIDTH];

void
tile_init(void)
{
  invalidTiles = 0;
  invalidFreeList = &invalidTileBuffers[0];
   invalidFreeList->prev = 0;
  //  invalidFreeList->next = 0;
  tile_redraw_t* ptr = invalidFreeList;
  for (int i = 1; i < MAX_INVALID_TILES; i++) {
      ptr->next = &invalidTileBuffers[i];
      ptr->next->prev = ptr;
      ptr = ptr->next;
  }


#if 0
  uint16_t* src = &background_tileAddresses[0][0];
  uint16_t* dest = &backgroundTiles[0][0];
  for (int i = 0; i < MAP_TILE_HEIGHT*MAP_TILE_WIDTH; i++) {
    *dest++ = *src++;
  }
#else
  disk_loadData(&backgroundTiles, &background_tileAddresses, sizeof(background_tileAddresses));
#endif
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
tile_invalidateTile(int x, int y, int offset)
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
  USE(fb);
  tile_redraw_t* ptr = invalidTiles;

  while (ptr != 0) {
    gfx_renderTile(fb, ptr->x, ptr->y, spriteFrameBuffer+ptr->offset);
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

  tile_tilePtr = &backgroundTiles[MAP_TILE_HEIGHT-1][MAP_TILE_WIDTH-1];
  for (int16_t y = SCREEN_HEIGHT-TILE_HEIGHT; y >= 0; y-=TILE_HEIGHT) {
    for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
      unsigned long offset = *tile_tilePtr;
      gfx_renderTileOffScreen(game_offScreenBuffer, x, y, spriteFrameBuffer+offset);
      gfx_renderTileOffScreen(game_onScreenBuffer, x, y, spriteFrameBuffer+offset);
      tile_tilePtr--;
    }
  }
  
  int y = FRAME_BUFFER_HEIGHT-TILE_HEIGHT;
  for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
    unsigned long offset = *tile_tilePtr;
    gfx_renderTileOffScreen(game_onScreenBuffer, x, y, spriteFrameBuffer+offset);
    gfx_renderTileOffScreen(game_offScreenBuffer, x, y, spriteFrameBuffer+offset);
    tile_tilePtr--;
  }
}


#ifndef INLINE_EVERYTHING
#include "tile_inlines.h"
#endif
