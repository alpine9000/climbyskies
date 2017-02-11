#include "game.h"

#define ITEM_MAX_ITEMS 100
#define ITEM_COLLISION_FUZZY 2

typedef enum {
  ITEM_ALIVE,
  ITEM_DEAD
} item_state_t;


typedef enum {
  ITEM_ANIM_COIN = 0,
} item_anim_t;


typedef struct item {
  struct item *prev;
  struct item *next;
  sprite_t sprite;
  sprite_save_t saves[2];
  sprite_animation_t* anim;
  item_anim_t animId;
  int frameCounter;
  item_state_t state;
  int deadRenderCount;
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
};

static item_t* item_activeList;
static item_t* item_freeList;
static int item_configIndex;
static int item_yIndex;
static item_t item_buffer[ITEM_MAX_ITEMS];


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

static void
item_add(int x, int y,int anim)
{
  item_t* ptr = item_getFree();
  ptr->state = ITEM_ALIVE;
  ptr->sprite.y = y;
  ptr->sprite.save = &ptr->saves[0];
  ptr->saves[0].blit[0].size = 0;
  ptr->saves[0].blit[1].size = 0;
  ptr->saves[1].blit[0].size = 0;
  ptr->saves[0].blit[1].size = 0;
  ptr->sprite.x = x;
  ptr->anim = &item_animations[anim];
  ptr->animId = anim;
  ptr->sprite.imageIndex = ptr->anim->animation.start;
  ptr->sprite.image = &sprite_imageAtlas[ptr->sprite.imageIndex];
  ptr->frameCounter = 0;
  item_addItem(ptr);
}


void
item_addCoin(uint32_t x, uint32_t y)
{
  item_add(x, y, ITEM_ANIM_COIN);
}


void
item_init(void)
{
  item_configIndex = 0;
  item_yIndex = 0;
  item_activeList = 0;
  item_freeList = &item_buffer[0];
  item_freeList->prev = 0;
  item_t* ptr = item_freeList;
  for (int i = 1; i < ITEM_MAX_ITEMS; i++) {
      ptr->next = &item_buffer[i];
      ptr->next->prev = ptr;
      ptr = ptr->next;
  }
}

INLINE void
item_save(frame_buffer_t fb, sprite_t* a)
{
  image_t* image = a->image;//&sprite_imageAtlas[a->imageIndex];
  int h = image->h;
  int y = a->y;
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
  if (y >= 0) {
    gfx_saveSprite16(fb, &a->save->blit[0], a->x, y, h);
    a->save->blit[1].size = 0;
  } else {
    if (y > -h) {
      gfx_saveSprite16(fb, &a->save->blit[0], a->x, 0, h+y);    
      gfx_saveSprite16(fb, &a->save->blit[1], a->x, FRAME_BUFFER_HEIGHT+y, -y);    
    } else {
      gfx_saveSprite16(fb, &a->save->blit[0], a->x, FRAME_BUFFER_HEIGHT+y,  h);    
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

  while (ptr != 0) {
    if (ptr->state != ITEM_DEAD) {
      sprite_render16NoShift(fb, ptr->sprite);
    }
    ptr = ptr->next;
  }
}

//static 
int
item_aabb(sprite_t* p, item_t* item)
{
  if ((p->x+(ITEM_COLLISION_FUZZY)) < (item->sprite.x+(ITEM_COLLISION_FUZZY)) + ITEM_WIDTH &&
      (p->x+(ITEM_COLLISION_FUZZY)) + PLAYER_WIDTH-(ITEM_COLLISION_FUZZY) > (item->sprite.x+(ITEM_COLLISION_FUZZY)) &&
      p->y < item->sprite.y + item->sprite.image->h &&
      PLAYER_HEIGHT + p->y > item->sprite.y) {
    return 1;
  }
  return 0;
}

#if 0
int
item_collision(sprite_t* p)
{
  item_t* ptr = item_activeList;

  while (ptr != 0) {
    if (item_aabb(p, ptr)) {
      return 1;
    }
    ptr = ptr->next;
  }

  return 0;
}
#endif


void
item_update(sprite_t* p)
{
  USE(p);
  int removedCount = 0;
  item_t* ptr = item_activeList;

  while (ptr != 0) {

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

    /*    if (ptr->state != ITEM_DEAD && item_aabb(p, ptr)) {
      //ptr->state = ITEM_DEAD;
      // ptr->deadRenderCount = 0;
      player_freeFall();
      }*/

    item_t* save = ptr;
    ptr = ptr->next;

    int remove = 0;

    if (save->state == ITEM_DEAD && (save->deadRenderCount++ > 2)) {
      remove = 1;
    } else  if (game_scrollCount == 0) {
      if ((save->sprite.y-game_cameraY) > (SCREEN_HEIGHT+ITEM_HEIGHT+1)) {
	remove = 1;
      } else if ((save->sprite.y-game_cameraY) < -(ITEM_HEIGHT+1)) {
	remove = 1;
      }
    }

    if (remove) {
      item_remove(save);
      item_addFree(save);
      removedCount++;
    }

  }
}
