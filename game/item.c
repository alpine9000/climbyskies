#include "game.h"

#define ITEM_MAX_ITEMS 12
#define ITEM_COLLISION_FUZZY 8

typedef enum {
  ITEM_ALIVE,
  ITEM_DEAD
} item_state_t;

typedef struct {
  uint8_t fb[(ITEM_WIDTH/8)*SCREEN_BIT_DEPTH*ITEM_MAX_HEIGHT];
} item_sprite_save_t;

typedef struct item {
  struct item *prev;
  struct item *next;
  sprite_t sprite;
  sprite_save_t saves[2];
  item_sprite_save_t saveBuffers[2];
  sprite_animation_t* anim;
  item_anim_t animId;
  int16_t frameCounter;
  item_state_t state;
  int16_t deadRenderCount;
  uint16_t* tilePtr;
} item_t;

static sprite_animation_t item_animations[] = {
  [ITEM_ANIM_COIN] = {
    .animation = {
    .start = SPRITE_COIN_1,
    .stop = SPRITE_COIN_14,
    .speed = 8
    },
    .facing = FACING_RIGHT
  },
#ifdef GAME_JETPACK
  [ITEM_ANIM_JETPACK] = {
    .animation = {
    .start = SPRITE_JETPACK,
    .stop = SPRITE_JETPACK,
    .speed = 0
    },
    .facing = FACING_RIGHT
  },
#endif
};

static int16_t item_count;
static item_t* item_activeList;
static item_t* item_freeList;
static __section(bss_c) item_t item_buffer[ITEM_MAX_ITEMS];


static void
item_updateItem(item_t* ptr);


static item_t*
item_getFree(void)
{
  item_t* entry = item_freeList;
  item_freeList = item_freeList->next;
  item_freeList->prev = 0;
  return entry;
}


static void
item_addFree(item_t* ptr)
{
  item_count--;
  if (item_freeList == 0) {
    item_freeList = ptr;
    ptr->next = 0;
    ptr->prev = 0;
  } else {
    ptr->next = item_freeList;
    ptr->next->prev = ptr;
    ptr->prev = 0;
    item_freeList = ptr;
  }
}


static void
item_addItem(item_t* ptr)
{
  item_count++;
  if (item_activeList == 0) {
    item_activeList = ptr;
    ptr->next = 0;
    ptr->prev = 0;
  } else {
    ptr->next = item_activeList;
    ptr->next->prev = ptr;
    ptr->prev = 0;
    item_activeList = ptr;
  }
}


static  void
item_remove(item_t* ptr)
{
  if (ptr->prev == 0) {
    item_activeList = ptr->next;
    if (item_activeList) {
      item_activeList->prev = 0;
    }
  } else {
    ptr->prev->next = ptr->next;
    if (ptr->next != 0) {
      ptr->next->prev = ptr->prev;
    }
  }
}


void
item_add(int16_t x, int16_t y, int16_t anim, uint16_t* tilePtr)
{
#ifndef GAME_JETPACK
  if (anim == ITEM_ANIM_JETPACK) {
    return;
  }
#endif
  if (item_count < ITEM_MAX_ITEMS) {
    item_t* ptr = item_getFree();
    ptr->tilePtr = tilePtr;
    ptr->state = ITEM_ALIVE;
    ptr->sprite.y = y;
    ptr->sprite.save = &ptr->saves[0];
    ptr->sprite.saveBuffer = &ptr->saveBuffers[0].fb[0];
    ptr->saves[0].blit[0].size = 0;
    ptr->saves[0].blit[1].size = 0;
    ptr->saves[1].blit[0].size = 0;
    ptr->saves[1].blit[1].size = 0;
    ptr->sprite.x = x;
    ptr->anim = &item_animations[anim];
    ptr->animId = anim;
    ptr->sprite.imageIndex = ptr->anim->animation.start;
    ptr->sprite.image = &sprite_imageAtlas[ptr->sprite.imageIndex];
    ptr->frameCounter = 0;
    item_addItem(ptr);
  }
}


void
item_init(void)
{
  item_count = 0;
  item_activeList = 0;
  item_freeList = &item_buffer[0];
  item_freeList->prev = 0;
  item_t* ptr = item_freeList;
  for (int16_t i = 1; i < ITEM_MAX_ITEMS; i++) {
      ptr->next = &item_buffer[i];
      ptr->next->prev = ptr;
      ptr = ptr->next;
  }
}


INLINE void
item_save(frame_buffer_t fb, sprite_t* a)
{
  image_t* image = a->image;
  int16_t h = image->h;
  int16_t y = a->y;
  if (y < game_cameraY) {
    h -= (game_cameraY - y);
    y += (game_cameraY - y);
  }

  if (y-game_cameraY + h > SCREEN_HEIGHT) {
    h -= (y-game_cameraY+h)-SCREEN_HEIGHT;
  }

  if (h <= 0) {
    a->save->blit[0].size = 0;
    a->save->blit[1].size = 0;
    return;
  }
  y = y-game_cameraY-game_screenScrollY;
  USE(fb);
  if (y >= 0) {
    gfx_saveSprite16(fb, a->saveBuffer, &a->save->blit[0], a->x, y, h);
    a->save->blit[1].size = 0;
  } else {
    if (y > -h) {
      gfx_saveSprite16(fb, a->saveBuffer, &a->save->blit[0], a->x, 0, h+y);
      frame_buffer_t dest = a->saveBuffer + ((h+y)*SCREEN_BIT_DEPTH*2); // TODO
      gfx_saveSprite16(fb, dest, &a->save->blit[1], a->x, FRAME_BUFFER_HEIGHT+y, -y);    
    } else {
      gfx_saveSprite16(fb, a->saveBuffer, &a->save->blit[0], a->x, FRAME_BUFFER_HEIGHT+y,  h);    
      a->save->blit[1].size = 0;
    }
  }
}


void
item_saveBackground(frame_buffer_t fb)
{
  item_t* ptr = item_activeList;

  while (ptr != 0) {
    item_save(fb, &ptr->sprite);
    ptr->sprite.save = ptr->sprite.save == &ptr->saves[0] ? &ptr->saves[1] : &ptr->saves[0];
    ptr->sprite.saveBuffer = ptr->sprite.saveBuffer == ptr->saveBuffers[0].fb ? ptr->saveBuffers[1].fb : ptr->saveBuffers[0].fb;
    ptr = ptr->next;
  }
}


void
item_restoreBackground(void)
{
  item_t* ptr = item_activeList;

  while (ptr != 0) {
    sprite_restore(ptr->sprite.save);
    ptr = ptr->next;
  }
}


void
item_render(frame_buffer_t fb)
{
  item_t* ptr = item_activeList;

  gfx_renderSprite16NoShiftSetup = 0;

  while (ptr != 0) {
    //item_updateItem(ptr);

    if (ptr->state != ITEM_DEAD) {
      sprite_render16NoShift(fb, ptr->sprite);
    }
    ptr = ptr->next;
  }
}


static inline int16_t
item_aabb(sprite_t* p, item_t* item)
{
#ifndef PLAYER_COLLISION_BOX
  if ((p->x+(ITEM_COLLISION_FUZZY)) < (item->sprite.x+(ITEM_COLLISION_FUZZY)) + (ITEM_WIDTH-ITEM_COLLISION_FUZZY) &&
      (p->x+(ITEM_COLLISION_FUZZY)) + PLAYER_WIDTH-(ITEM_COLLISION_FUZZY) > (item->sprite.x+(ITEM_COLLISION_FUZZY)) &&
      p->y < item->sprite.y + item->sprite.image->h &&
      PLAYER_HEIGHT + p->y > item->sprite.y) {
    return 1;
  }
  return 0;
#else
  if ((p->collisionBox.x1 < (item->sprite.x+(ITEM_COLLISION_FUZZY)) + (ITEM_WIDTH-ITEM_COLLISION_FUZZY)) &&
      (p->collisionBox.x2 > (item->sprite.x+(ITEM_COLLISION_FUZZY))) &&
      (p->collisionBox.y1 < (item->sprite.y + item->sprite.image->h)) &&
      (p->collisionBox.y2 > item->sprite.y)) {
    return 1;
  }
  return 0;
#endif
}


static void
item_updateItem(item_t* ptr)
{
  if (ptr->frameCounter == ptr->anim->animation.speed) {
    ptr->sprite.imageIndex++;
    ptr->frameCounter = 0;
    if (ptr->sprite.imageIndex > ptr->anim->animation.stop) {
      ptr->sprite.imageIndex = ptr->anim->animation.start;
    }
    ptr->sprite.image = &sprite_imageAtlas[ptr->sprite.imageIndex];
  } else {
    ptr->frameCounter++;
  }
  
  if (/*(ptr->frameCounter == 0) &&*/ ptr->state != ITEM_DEAD && item_aabb(&player.sprite, ptr)) {
    switch (ptr->animId) {
    case ITEM_ANIM_COIN:
      game_score += 100;
      sound_queueSound(SOUND_PICKUP);
      ptr->state = ITEM_DEAD;
      *ptr->tilePtr = 0;
      ptr->deadRenderCount = 0;
      break;
#ifdef GAME_JETPACK
    case ITEM_ANIM_JETPACK:
      if (player.jetpackFuel == 0) {
	sound_queueSound(SOUND_PICKUP);
	player.jetpackFuel += 100;
      }
      break;
#endif
    default:
      break;
    }
  }
}

void
item_update(void)
{
  item_t* ptr = item_activeList;

  while (ptr != 0) {
    item_updateItem(ptr);
    item_t* save = ptr;
    ptr = ptr->next;
    
    int16_t remove = 0;
    
    if (save->state == ITEM_DEAD && (save->deadRenderCount++ > 2)) {
      remove = 1;
    } else {
      if ((save->sprite.y-game_cameraY) > (SCREEN_HEIGHT+(TILE_HEIGHT*2))) {
	remove = 1;
      } else if ((save->sprite.y-game_cameraY) < -((TILE_HEIGHT*2))) {
	remove = 1;
      }
    }
    
    if (remove) {
      item_remove(save);
      item_addFree(save);
    }    
  }
}

#ifdef DEBUG
int16_t 
item_getCount(void)
{
  return item_count;
}
#endif
