#include "game.h"

void 
tile_renderScreen(frame_buffer_t frameBuffer, uint16_t screenY)
{
  extern unsigned short background_tileAddresses[100][14];
  unsigned short* tilePtr = &background_tileAddresses[99][13];
  //for (uint16_t i = 0, y = 0; y < SCREEN_HEIGHT; y+=16) {
  for (int16_t i = 0, y = SCREEN_HEIGHT-16; y >= 0; y-=16) {
    for (int16_t x = SCREEN_WIDTH-16; x >=0; x-=16) {
      gfx_renderTile2(frameBuffer, x, y, spriteFrameBuffer+*tilePtr--);
    }
  }
}
