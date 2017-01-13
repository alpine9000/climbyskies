#include "game.h"


typedef struct {
  int x;
  int y;
  int lastX;
  int lastY;
  int lastScrollY;
  int bobIndex;
  int deltaX;
  int deltaY;
} cloud_t;

static
cloud_t cloud = {
//  .x = SCREEN_WIDTH/2,
  .x = -0,
  //  .y = WORLD_HEIGHT-200,
  .y = WORLD_HEIGHT-SCREEN_HEIGHT-28,
  .bobIndex = BOB_CLOUD_1,
  .deltaX = 0,
  .deltaY = 0  
};


void
cloud_init(frame_buffer_t fb)
{
  cloud.lastX = cloud.x;
  cloud.lastY = cloud.y;
  cloud.lastScrollY = screenScrollY;
  cloud_saveBackground(fb);
  cloud_render(fb);
}

void
cloud_saveBackground(frame_buffer_t fb)
{
  if (cloud.y < cameraY+SCREEN_HEIGHT) {
    bob_save(fb, cloud.x, cloud.y, cloud.bobIndex);
  }
}


void
cloud_restoreBackground(frame_buffer_t fb)
{
  if (cloud.lastY < cameraY+SCREEN_HEIGHT) {
    bob_clear(fb, cloud.lastX, cloud.lastY, cloud.bobIndex, cloud.lastScrollY);
  }
}


void
tile_render(frame_buffer_t fb, int16_t x, int16_t y, frame_buffer_t tile)
{
  y = y-cameraY-screenScrollY;
  if (y >= 0) {
    gfx_renderTile3(fb, x, y, 16, tile);
  } else {
    if (y > -16) {
      gfx_renderTile3(fb, x, y, 16+y, tile);
      gfx_renderTile3(fb, x, FRAME_BUFFER_HEIGHT+y, -y, tile);
    } else {
      gfx_renderTile3(fb, x, FRAME_BUFFER_HEIGHT+y, 16, tile);
    }
  }
}

void
cloud_render(frame_buffer_t fb)
{
  if (cloud.y < cameraY+SCREEN_HEIGHT) {
    bob_render(fb, cloud.x, cloud.y, cloud.bobIndex);
    int py = (cloud.y/TILE_HEIGHT);
    int px = (cloud.x/TILE_WIDTH);
    for (int x = 0; x < 4; x++) {
      for (int y = 0; y < 3; y++) {
	int tile = background_tileAddresses[py+y][px+x];
	if (tile != 0) {
	  tile_render(fb, (px+x)*16, (py+y)*16, spriteFrameBuffer+tile);
	}
      }
    }
    cloud.lastX = cloud.x;
    cloud.lastY = cloud.y;
    cloud.lastScrollY = screenScrollY;
  }
}

void
cloud_update(void)
{
  if (scrollCount > 0) {
    cloud.y--;
  }

  if (cloud.y >= cameraY+SCREEN_HEIGHT) {
     cloud.y = cameraY-32;
    //cloud.x = 200;
  }
}
