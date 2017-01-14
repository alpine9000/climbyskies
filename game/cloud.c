#include "game.h"

#define CLOUD_HEIGHT 32
#define CLOUD_WIDTH 16*3
#define NUM_CLOUDS 3

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
int16_t cloudX[] = {
  0, 
  64, 
  SCREEN_WIDTH-CLOUD_WIDTH,
  16,
  SCREEN_WIDTH-CLOUD_WIDTH-16,
  64-16,
  16,
  ((SCREEN_WIDTH/2+CLOUD_WIDTH/2)/16)*16,
  -1
};

static int cloudXIndex = 0;

static
cloud_t clouds[NUM_CLOUDS] = {
  {
    .x = 0,
    .y = WORLD_HEIGHT-SCREEN_HEIGHT+32,
    .bobIndex = BOB_CLOUD_1,
    .deltaX = 0,
    .deltaY = 0  
  },
  {
    .x = 64,
    .y = WORLD_HEIGHT-(SCREEN_HEIGHT/4),
    .bobIndex = BOB_CLOUD_1,
    .deltaX = 0,
    .deltaY = 0  
  },
  {
    .x = SCREEN_WIDTH-CLOUD_WIDTH,
    .y = WORLD_HEIGHT-SCREEN_HEIGHT+64,
    .bobIndex = BOB_CLOUD_2,
    .deltaX = 0,
    .deltaY = 0  
  }
};


void
cloud_init(frame_buffer_t fb)
{
  for (int i = 0; i < NUM_CLOUDS; i++) {
    cloud_t* cloud = &clouds[i];
    cloud->saves[0].blit[0].size = 0;
    cloud->saves[0].blit[1].size = 0;
    cloud->saves[1].blit[0].size = 0;
    cloud->saves[1].blit[1].size = 0;
    cloud->save = &cloud->saves[0];
  }
  
  cloud_saveBackground(fb);
  cloud_render(fb);
}

void
cloud_saveBackground(frame_buffer_t fb)
{
  for (int i = 0; i < NUM_CLOUDS; i++) {  
    cloud_t* cloud = &clouds[i];
    bob_save(fb, cloud->x, cloud->y, cloud->bobIndex, cloud->save);  
    cloud->save = cloud->save == &cloud->saves[0] ? &cloud->saves[1] : &cloud->saves[0];
  }
}


void
cloud_restoreBackground(void)
{
  for (int i = 0; i < NUM_CLOUDS; i++) {  
    cloud_t* cloud = &clouds[i];
    bob_clear(cloud->save);
  }
}



void
cloud_render(frame_buffer_t fb)
{
  for (int i = 0; i < NUM_CLOUDS; i++) {
    cloud_t* cloud = &clouds[i];
    bob_renderNoMask(fb, cloud->x, cloud->y, cloud->bobIndex);
    int py = (cloud->y>>4); // (cloud->y/TILE_HEIGHT);
    int px = (cloud->x>>4); // (cloud->x/TILE_WIDTH);
    for (int x = 0; x < 3; x++) {
      if (px+x < MAP_TILE_WIDTH) {
	for (int y = 0; y < 3; y++) {	  
	  int tile = background_tileAddresses[py+y][px+x];
	  if (tile != 0) {
	    gfx_renderTile4(fb, (px+x)<<4, (py+y)<<4, spriteFrameBuffer+tile);
	  }
	}
      }
    }
  }
}

void
cloud_update(void)
{
  for (int i = 0; i < NUM_CLOUDS; i++) {
    cloud_t* cloud = &clouds[i];
    if (scrollCount > 0) {
      cloud->y--;
    }
    
    if (cloud->y >= cameraY+SCREEN_HEIGHT) {
      cloud->y = cameraY-CLOUD_HEIGHT;
      if (cloudX[cloudXIndex] == -1) {
	cloudXIndex = 0;
      }
      cloud->x = cloudX[cloudXIndex];
      cloudXIndex++;
    }
  }
}
