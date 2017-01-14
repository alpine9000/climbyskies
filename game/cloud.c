#include "game.h"

#define CLOUD_HEIGHT 32

typedef struct {
  int x;
  int y;
  int bobIndex;
  int deltaX;
  int deltaY;
  bob_save_t* save;
  bob_save_t saves[2];
} cloud_t;

static
cloud_t cloud = {
//  .x = SCREEN_WIDTH/2,
  .x = -0,
  //  .y = WORLD_HEIGHT-200,
  .y = WORLD_HEIGHT-SCREEN_HEIGHT+180,
  .bobIndex = BOB_CLOUD_1,
  .deltaX = 0,
  .deltaY = 0  
};


void
cloud_init(frame_buffer_t fb)
{
  cloud.saves[0].blit[0].size = 0;
  cloud.saves[0].blit[1].size = 0;
  cloud.saves[1].blit[0].size = 0;
  cloud.saves[1].blit[1].size = 0;
  cloud.save = &cloud.saves[0];

  
  cloud_saveBackground(fb);
  /*saveBuffer = saveBuffer == saveBuffer1 ? saveBuffer2 : saveBuffer1;    
  cloud_saveBackground(fb);
  saveBuffer = saveBuffer == saveBuffer1 ? saveBuffer2 : saveBuffer1;    */

  cloud_render(fb);
}

void
cloud_saveBackground(frame_buffer_t fb)
{
  bob_save(fb, cloud.x, cloud.y, cloud.bobIndex, cloud.save);
  
  cloud.save = cloud.save == &cloud.saves[0] ? &cloud.saves[1] : &cloud.saves[0];
}


void
cloud_restoreBackground(void)
{

    bob_clear(cloud.save);

      //  }
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
