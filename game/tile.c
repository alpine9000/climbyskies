#include "game.h"


extern unsigned short background_tileAddresses[MAP_TILE_HEIGHT][MAP_TILE_WIDTH];
static unsigned short* tilePtr;
static int tileX;

void 
tile_renderScreen(void)
{

  tileX = SCREEN_WIDTH-TILE_WIDTH;

  tilePtr = &background_tileAddresses[MAP_TILE_HEIGHT-1][MAP_TILE_WIDTH-1];
  for (int16_t y = SCREEN_HEIGHT-TILE_HEIGHT; y >= 0; y-=TILE_HEIGHT) {
    for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
      gfx_renderTile2(offScreenBuffer, x, y, spriteFrameBuffer+*tilePtr);
      gfx_renderTile2(onScreenBuffer, x, y, spriteFrameBuffer+*tilePtr);
      tilePtr--;
    }
  }
  
  int y = FRAME_BUFFER_HEIGHT-TILE_HEIGHT;
  for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
    gfx_renderTile2(onScreenBuffer, x, y, spriteFrameBuffer+*tilePtr);
    gfx_renderTile2(offScreenBuffer, x, y, spriteFrameBuffer+*tilePtr);
    tilePtr--;
  }
}


uint32_t
tile_renderNextTile(uint16_t hscroll)
{
  int y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

  gfx_renderTile2(offScreenBuffer, tileX, y, spriteFrameBuffer+*tilePtr);
  gfx_renderTile2(onScreenBuffer, tileX, y, spriteFrameBuffer+*tilePtr);
  
  tilePtr = tilePtr-1;

  tileX -= TILE_WIDTH;

  if (tileX < 0) {
    tileX = SCREEN_WIDTH-TILE_WIDTH;
  }

  return tilePtr <= &background_tileAddresses[0][0];
}
