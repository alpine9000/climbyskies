#include "game.h"

extern unsigned short background_tileAddresses[100][14];
static unsigned short* tilePtr;

void 
tile_renderScreen(frame_buffer_t frameBuffer)
{

  tilePtr = &background_tileAddresses[99][13];
  for (int16_t i = 0, y = SCREEN_HEIGHT-TILE_HEIGHT; y >= 0; y-=TILE_HEIGHT) {
    for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
      gfx_renderTile2(frameBuffer, x, y, spriteFrameBuffer+*tilePtr--);
    }
  }
  
  int y = FRAME_BUFFER_HEIGHT-TILE_HEIGHT;
  for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
    gfx_renderTile2(frameBuffer, x, y, spriteFrameBuffer+*tilePtr--);
  }
}


uint32_t
tile_renderNextRow(frame_buffer_t frameBuffer, uint16_t hscroll)
{
  int y = FRAME_BUFFER_HEIGHT-TILE_HEIGHT-hscroll;
  for (int16_t x = SCREEN_WIDTH-TILE_WIDTH; x >=0; x-=TILE_WIDTH) {
    gfx_renderTile2(frameBuffer, x, y, spriteFrameBuffer+*tilePtr--);
  }

  return tilePtr <= &background_tileAddresses[0][0];
}


uint32_t
tile_renderNextTile(frame_buffer_t frameBuffer, uint16_t hscroll)
{
  int y = (FRAME_BUFFER_HEIGHT-hscroll-(2*TILE_HEIGHT));
  static int x = SCREEN_WIDTH-TILE_WIDTH;

  if (y < 0) {
    y = FRAME_BUFFER_HEIGHT+y;
  }

  gfx_renderTile2(frameBuffer, x, y, spriteFrameBuffer+*tilePtr--);
  
  x -= TILE_WIDTH;

  if (x < 0) {
    x = SCREEN_WIDTH-TILE_WIDTH;
  }

  return tilePtr <= &background_tileAddresses[0][0];
}
