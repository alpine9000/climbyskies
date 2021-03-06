#include "game.h"

#define CLOUD_NUM_CLOUDS 3

typedef struct {
  uint8_t fb[(CLOUD_WIDTH/8)*SCREEN_BIT_DEPTH*CLOUD_HEIGHT];
} cloud_sprite_save_t;

typedef struct {
  sprite_t sprite;
  int16_t deltaX;
  int16_t deltaY;
  sprite_save_t saves[2];
  cloud_sprite_save_t saveBuffers[2];
} cloud_t;

static uint16_t cloud_sizeLUT[65];
uint16_t cloud_spriteRenderSetup;

static int16_t cloudX[] = {
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

static int16_t cloudXIndex = 0;

static __section(random_c) cloud_t clouds[CLOUD_NUM_CLOUDS];

static cloud_t _clouds[CLOUD_NUM_CLOUDS] = {
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

  for (uint16_t h = 0; h <= 64; h++) {
    cloud_sizeLUT[h] = (h)<<6 | (CLOUD_WIDTH/16);;
  }

#if 1
    memcpy(clouds, _clouds, sizeof(clouds));
    for (int16_t i = 0; i < CLOUD_NUM_CLOUDS; i++) {
      cloud_t* cloud = &clouds[i];
      cloud->sprite.save = &cloud->saves[0];
      cloud->sprite.saveBuffer = cloud->saveBuffers[0].fb;
    }
#else
    // crashes with -mregparm=2 and vasm default optimisations
      for (int16_t i = 0; i < CLOUD_NUM_CLOUDS; i++) {
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
  for (int16_t i = 0; i < CLOUD_NUM_CLOUDS; i++) {  
    cloud_t* cloud = &clouds[i];
    cloud_save(fb, &cloud->sprite);
    cloud->sprite.save = cloud->sprite.save == &cloud->saves[0] ? &cloud->saves[1] : &cloud->saves[0];
    cloud->sprite.saveBuffer = cloud->sprite.saveBuffer == cloud->saveBuffers[0].fb ? cloud->saveBuffers[1].fb : cloud->saveBuffers[0].fb;    
  }
}


static inline void
cloud_restoreSprite(gfx_blit_t* blit)
{
  volatile struct Custom* _custom = CUSTOM;

  hw_waitBlitter();

  _custom->bltamod = 0;
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
  volatile struct Custom* _custom = CUSTOM;
  hw_waitBlitter();

  _custom->bltcon0 = (SRCA|DEST|0xf0);
  _custom->bltcon1 = 0;
  //_custom->bltafwm = 0xffff;
  _custom->bltalwm = 0xffff;

  for (int16_t i = 0; i < CLOUD_NUM_CLOUDS; i++) {  
    cloud_t* cloud = &clouds[i];
    cloud_restore(cloud->sprite.save);
  }
}


void
cloud_render(frame_buffer_t fb)
{
  cloud_initSpriteRender();
  for (int16_t i = 0; i < CLOUD_NUM_CLOUDS; i++) {
    cloud_t* cloud = &clouds[i];
    cloud_spriteRender(fb, &cloud->sprite);
  }

  int16_t mask = -1;
  
  for (int16_t i = 0; i < CLOUD_NUM_CLOUDS; i++) {
    cloud_t* cloud = &clouds[i];
    int16_t py = (cloud->sprite.y>>4); // (cloud->sprite.y/TILE_HEIGHT);
    int16_t px = (cloud->sprite.x>>4); // (cloud->sprite.x/TILE_WIDTH);

    for (int16_t x = 0; x < 3; x++) {
      for (int16_t y = 0; y < 3; y++) {	  
	uint16_t tile = level.tileAddresses[py+y][px+x];
	if (tile != TILE_SKY) {
	  if (tile > TILE_MASKED_BLIT_RANGE) {
	    if (mask != 1) {
	      cloud_setupRenderPartialTileMask();
	      mask = 1;
	    }
	    cloud_renderTileMask(fb, (px+x)<<4/* *TILE_WIDTH */, (py+y)<<4 /* *TILE_HEIGHT */, tile);
	  } else {
	    if (mask != 0) {
	      cloud_setupRenderPartialTile();
	      mask = 0;
	    }
	    cloud_renderTile(fb, (px+x)<<4/* *TILE_WIDTH */, (py+y)<<4 /* *TILE_HEIGHT */, level.spriteBitplanes+tile);
	  }
	}
      }
    }
  }
}


void
cloud_update(int16_t scroll)
{
  if (game_cameraY >= WORLD_HEIGHT-SCREEN_HEIGHT) {
    return;
  }
  for (int16_t i = 0; i < CLOUD_NUM_CLOUDS; i++) {
    cloud_t* cloud = &clouds[i];

    //if (game_scroll) {
    //      cloud->sprite.y-=(game_scroll>>2 /* /4 */);
    //}
    cloud->sprite.y -= scroll;
    
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
