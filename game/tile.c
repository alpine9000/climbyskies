#include "game.h"

#define MAX_INVALID_TILES 100

extern unsigned short background_tileAddresses[MAP_TILE_HEIGHT][MAP_TILE_WIDTH];
static unsigned short* tilePtr;
static int tileX;
static tile_redraw_t* invalidTiles = 0;
static tile_redraw_t* invalidFreeList = 0;
static tile_redraw_t invalidTileBuffers[MAX_INVALID_TILES];


void
tile_init(void)
{
  invalidTiles = 0;
  invalidFreeList = &invalidTileBuffers[0];
  invalidFreeList->prev = 0;
  invalidFreeList->next = 0;
  tile_redraw_t* ptr = invalidFreeList;
  for (int i = 1; i < MAX_INVALID_TILES; i++) {
      ptr->next = &invalidTileBuffers[i];
      ptr->next->prev = ptr;
      ptr = ptr->next;
  }
}


static tile_redraw_t*
tile_getFree(void)
{
  tile_redraw_t* entry = invalidFreeList;
  invalidFreeList = invalidFreeList->next;
  return entry;
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


static void
tile_removeInvalid(tile_redraw_t* ptr)
{
  if (ptr->prev == 0) {
    invalidTiles = ptr->next;
    invalidTiles->prev = 0;
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
  tile_redraw_t* ptr = invalidTiles;

  while (ptr != 0) {
    gfx_renderTile(fb, ptr->x, ptr->y, spriteFrameBuffer+ptr->offset);
    ptr->count--;
    if (ptr->count == 0) {
      tile_removeInvalid(ptr);
    }
    ptr = ptr->next;
  }
}


void 
tile_renderScreen(void)
{
  tileX = SCREEN_WIDTH-TILE_WIDTH;

  tilePtr = &background_tileAddresses[MAP_TILE_HEIGHT-1][MAP_TILE_WIDTH-1];
  for (int16_t y = SCREEN_HEIGHT-TILE_HEIGHT; y >= 0; y-=TILE_HEIGHT) {
    for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
      gfx_renderTileOffScreen(offScreenBuffer, x, y, spriteFrameBuffer+*tilePtr);
      gfx_renderTileOffScreen(onScreenBuffer, x, y, spriteFrameBuffer+*tilePtr);
      tilePtr--;
    }
  }
  
  int y = FRAME_BUFFER_HEIGHT-TILE_HEIGHT;
  for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
    gfx_renderTileOffScreen(onScreenBuffer, x, y, spriteFrameBuffer+*tilePtr);
    gfx_renderTileOffScreen(offScreenBuffer, x, y, spriteFrameBuffer+*tilePtr);
    tilePtr--;
  }
}


void
tile_renderNextTile(uint16_t hscroll)
{
  int y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

  gfx_renderTileOffScreen(offScreenBuffer, tileX, y, spriteFrameBuffer+*tilePtr);
  gfx_renderTileOffScreen(onScreenBuffer, tileX, y, spriteFrameBuffer+*tilePtr);
  
  tilePtr = tilePtr-1;

  tileX -= TILE_WIDTH;

  if (tileX < 0) {
    tileX = SCREEN_WIDTH-TILE_WIDTH;
  }
}


void
tile_renderNextTileDown(uint16_t hscroll)
{
  int y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

  tilePtr = tilePtr+1;
  tileX += TILE_WIDTH;

  if (tileX > SCREEN_WIDTH-TILE_WIDTH) {
    tileX = 0;
  }

#define OFFSET (((FRAME_BUFFER_HEIGHT-(1*TILE_HEIGHT))/TILE_HEIGHT)*(SCREEN_WIDTH/TILE_WIDTH))
  gfx_renderTileOffScreen(offScreenBuffer, tileX, y, spriteFrameBuffer+*(tilePtr+OFFSET));
  gfx_renderTileOffScreen(onScreenBuffer, tileX, y, spriteFrameBuffer+*(tilePtr+OFFSET));
}
