#include "game.h"


#define NUM_CLOUDS 3

typedef struct {
  sprite_t sprite;
  int deltaX;
  int deltaY;
  sprite_save_t saves[2];
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

static cloud_t clouds[NUM_CLOUDS];

static cloud_t _clouds[NUM_CLOUDS] = {
  {
    .sprite = {
      .x = 0,
      .y = WORLD_HEIGHT-SCREEN_HEIGHT+32,
      .imageIndex = SPRITE_CLOUD_1
    },
    .deltaX = 0,
    .deltaY = 0  
  },
  {
    .sprite = {
      .x = 64,
      .y = WORLD_HEIGHT-(SCREEN_HEIGHT/4),
      .imageIndex = SPRITE_CLOUD_1
    },
    .deltaX = 0,
    .deltaY = 0  
  },
  {
    .sprite = {
      .x = SCREEN_WIDTH-CLOUD_WIDTH,
      .y = WORLD_HEIGHT-SCREEN_HEIGHT+64,
      .imageIndex = SPRITE_CLOUD_2
    },
    .deltaX = 0,
    .deltaY = 0  
  }
};

#include "cloud_inlines.h"

void
cloud_init(void)
{
  cloudXIndex = 0;

#if 1
    memcpy(clouds, _clouds, sizeof(clouds));
    for (int i = 0; i < NUM_CLOUDS; i++) {
      cloud_t* cloud = &clouds[i];
      cloud->sprite.save = &cloud->saves[0];
    }
#else
    // crashes with -mregparm=2 and vasm default optimisations
      for (int i = 0; i < NUM_CLOUDS; i++) {
	cloud_t* cloud = &clouds[i];
        clouds[i] = _clouds[i];
#if 1
	cloud->saves[0].blit[0].size = 0;
	cloud->saves[0].blit[1].size = 0;
	cloud->saves[1].blit[0].size = 0;
	cloud->saves[1].blit[1].size = 0;
#endif
	cloud->sprite.save = &cloud->saves[0];
      }
#endif


}


void
cloud_saveBackground(frame_buffer_t fb)
{
#if 0 // Slower
  volatile struct Custom* _custom = CUSTOM;
  // Risky?
   hw_waitBlitter();

  _custom->bltcon0 = (SRCA|DEST|0xf0);
  _custom->bltcon1 = 0;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;
#endif

  for (int i = 0; i < NUM_CLOUDS; i++) {  
    cloud_t* cloud = &clouds[i];
    cloud_save(fb, &cloud->sprite);
    cloud->sprite.save = cloud->sprite.save == &cloud->saves[0] ? &cloud->saves[1] : &cloud->saves[0];
  }
}


static inline void
cloud_restoreSprite(gfx_blit_t* blit)
{
  volatile struct Custom* _custom = CUSTOM;

  hw_waitBlitter();

  _custom->bltamod = blit->mod;
  _custom->bltdmod = blit->mod;
  _custom->bltapt = (uint8_t*)blit->dest;
  _custom->bltdpt = (uint8_t*)blit->source;
  _custom->bltsize = blit->size;
}


static inline void
cloud_restore(sprite_save_t* save)
{
  if (save->blit[0].size > 0) {
    cloud_restoreSprite(&save->blit[0]);
  }

  if (save->blit[1].size > 0) {
    cloud_restoreSprite(&save->blit[1]);
  }
}


void
cloud_restoreBackground(void)
{
#if 1
  volatile struct Custom* _custom = CUSTOM;
  hw_waitBlitter();
  _custom->bltcon0 = (SRCA|DEST|0xf0);
  _custom->bltcon1 = 0;
  _custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;
#endif

  for (int i = 0; i < NUM_CLOUDS; i++) {  
    cloud_t* cloud = &clouds[i];
    cloud_restore(cloud->sprite.save);
  }
}


void
cloud_render(frame_buffer_t fb)
{
  cloud_setupRenderSpriteNoMask();
  for (int i = 0; i < NUM_CLOUDS; i++) {
    cloud_t* cloud = &clouds[i];
    cloud_spriteRender(fb, &cloud->sprite);
  }

  cloud_setupRenderPartialTile();
  for (int i = 0; i < NUM_CLOUDS; i++) {
    cloud_t* cloud = &clouds[i];
    int py = (cloud->sprite.y>>4); // (cloud->sprite.y/TILE_HEIGHT);
    int px = (cloud->sprite.x>>4); // (cloud->sprite.x/TILE_WIDTH);

    for (int x = 0; x < 3; x++) {
      if (px+x < MAP_TILE_WIDTH) {
	for (int y = 0; y < 3; y++) {	  
	  int tile = backgroundTiles[py+y][px+x];
	  if (tile != TILE_SKY) {
	    cloud_renderTile(fb, (px+x)<<4/* *TILE_WIDTH */, (py+y)<<4 /* *TILE_HEIGHT */, spriteFrameBuffer+tile);
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
    if (game_scrollCount > 0) {
      cloud->sprite.y-=(game_scroll>>2 /* /4 */);
    }
    
    if (cloud->sprite.y >= game_cameraY+SCREEN_HEIGHT) {
      cloud->sprite.y = game_cameraY-CLOUD_HEIGHT;
      if (cloudX[cloudXIndex] == -1) {
	cloudXIndex = 0;
      }
      cloud->sprite.x = cloudX[cloudXIndex];
      cloudXIndex++;
    }

    if (cloud->sprite.y < game_cameraY-CLOUD_HEIGHT) {
      cloud->sprite.y = game_cameraY+SCREEN_HEIGHT-1;
      if (cloudX[cloudXIndex] == -1) {
        cloudXIndex = 0;
      }
      cloud->sprite.x = cloudX[cloudXIndex];
      cloudXIndex++;
    }
  }
}
