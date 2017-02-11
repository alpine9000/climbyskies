#include "game.h"

#define ENEMY_MAX_ENEMIES 10
#define ENEMY_COLLISION_FUZZY 16
#define ENEMY_MAX_CONFIGS 6
#define ENEMY_MAX_Y 3

typedef enum {
  ENEMY_ALIVE,
  ENEMY_DEAD
} enemy_state_t;


typedef enum {
  ENEMY_ANIM_RIGHT_RUN = 0,
  ENEMY_ANIM_LEFT_RUN,
  ENEMY_ANIM_RIGHT_SKATE,
  ENEMY_ANIM_LEFT_SKATE,
  ENEMY_ANIM_RIGHT_DRAGON
} enemy_anim_t;


typedef struct enemy {
  struct enemy *prev;
  struct enemy *next;
  sprite_t sprite;
  velocity_t velocity;
  sprite_save_t saves[2];
  sprite_animation_t* anim;
  enemy_anim_t animId;
  int frameCounter;
  int width;
  enemy_state_t state;
  int deadRenderCount;
  int onGround;
} enemy_t;

static sprite_animation_t enemy_animations[] = {
  [ENEMY_ANIM_RIGHT_RUN] = {
    .animation = {
    .start = SPRITE_ENEMY_RUN_RIGHT_1,
    .stop = SPRITE_ENEMY_RUN_RIGHT_4,
    .speed = 4
    },
    .facing = FACING_RIGHT
  },
  [ENEMY_ANIM_LEFT_RUN] = {
    .animation = {
      .start = SPRITE_ENEMY_RUN_LEFT_1, 
      .stop = SPRITE_ENEMY_RUN_LEFT_4,
      .speed = 4
    },
    .facing = FACING_LEFT
  },
  [ENEMY_ANIM_RIGHT_SKATE] = {
    .animation = {
    .start = SPRITE_ENEMY_SKATE_RIGHT_1,
    .stop = SPRITE_ENEMY_SKATE_RIGHT_2,
    .speed = 4
    },
    .facing = FACING_RIGHT
  },
  [ENEMY_ANIM_LEFT_SKATE] = {
    .animation = {
      .start = SPRITE_ENEMY_SKATE_LEFT_1, 
      .stop = SPRITE_ENEMY_SKATE_LEFT_2,
      .speed = 4
    },
    .facing = FACING_LEFT
  },
  [ENEMY_ANIM_RIGHT_DRAGON] = {
    .animation = {
      .start = SPRITE_ENEMY_DRAGON_RIGHT_1,
      .stop = SPRITE_ENEMY_DRAGON_RIGHT_4,
      .speed = 4
    },
    .facing = FACING_RIGHT
  }
};

static enemy_t* enemy_activeList;
static enemy_t* enemy_freeList;
static int enemy_configIndex;
static int enemy_yIndex;
static uint16_t enemy_yStarts[WORLD_HEIGHT];
static enemy_t enemy_buffer[ENEMY_MAX_ENEMIES];

typedef struct {
  int x;
  int onGround;
  int height;
  enemy_anim_t anim;
} enemy_config_t;

static int enemy_y[ENEMY_MAX_Y] = {
  0,
  97,
  193
};

static enemy_config_t enemy_configs[ENEMY_MAX_CONFIGS] = {
  {
    .x = -32,
    .onGround = 1,
    .height = PLAYER_HEIGHT,
    .anim = ENEMY_ANIM_RIGHT_RUN
  },
  {
    .x = SCREEN_WIDTH,
    .onGround = 1,
    .height = PLAYER_HEIGHT,
    .anim = ENEMY_ANIM_LEFT_SKATE
  },
  {
    .x = -32,
    .onGround = 0,
    .height = 25,
    .anim = ENEMY_ANIM_RIGHT_DRAGON
  },
  {
    .x = -32,
    .onGround = 1,
    .height = PLAYER_HEIGHT,
    .anim = ENEMY_ANIM_RIGHT_SKATE
  },
  {
    .x = SCREEN_WIDTH,
    .onGround = 1,
    .height = PLAYER_HEIGHT,
    .anim = ENEMY_ANIM_LEFT_RUN
  },
  {
    .x = -32,
    .onGround = 1,
    .height = PLAYER_HEIGHT,
    .anim = ENEMY_ANIM_RIGHT_RUN
  }
  
};


static enemy_t*
enemy_getFree(void)
{
  enemy_t* entry = enemy_freeList;
  enemy_freeList = enemy_freeList->next;
  enemy_freeList->prev = 0;
  return entry;
}

static void
enemy_addFree(enemy_t* ptr)
{
  if (enemy_freeList == 0) {
    enemy_freeList = ptr;
    ptr->next = 0;
    ptr->prev = 0;
  } else {
    ptr->next = enemy_freeList;
    ptr->next->prev = ptr;
    ptr->prev = 0;
    enemy_freeList = ptr;
  }
}

static void
enemy_addEnemy(enemy_t* ptr)
{
  if (enemy_activeList == 0) {
    enemy_activeList = ptr;
    ptr->next = 0;
    ptr->prev = 0;
  } else {
    ptr->next = enemy_activeList;
    ptr->next->prev = ptr;
    ptr->prev = 0;
    enemy_activeList = ptr;
  }
}


static  void
enemy_remove(enemy_t* ptr)
{
  if (ptr->prev == 0) {
    enemy_activeList = ptr->next;
    if (enemy_activeList) {
      enemy_activeList->prev = 0;
    }
  } else {
    ptr->prev->next = ptr->next;
    if (ptr->next != 0) {
      ptr->next->prev = ptr->prev;
    }
  }
}

static void
enemy_add(int x, int y, int onGround, int anim)
{
  enemy_t* ptr = enemy_getFree();
  ptr->state = ENEMY_ALIVE;
  ptr->width = 32-(ENEMY_COLLISION_FUZZY);
  ptr->onGround = onGround;
  ptr->sprite.y = y;
  ptr->sprite.save = &ptr->saves[0];
  ptr->saves[0].blit[0].size = 0;
  ptr->saves[0].blit[1].size = 0;
  ptr->saves[1].blit[0].size = 0;
  ptr->saves[0].blit[1].size = 0;
  ptr->sprite.x = x;
  if (anim == ENEMY_ANIM_LEFT_RUN || anim == ENEMY_ANIM_LEFT_SKATE) {
    ptr->velocity.x = -1;
  } else {
    ptr->velocity.x = 1;
  }
  ptr->velocity.y = 0;
  ptr->anim = &enemy_animations[anim];
  ptr->animId = anim;
  ptr->sprite.imageIndex = ptr->anim->animation.start;
  ptr->sprite.image = &sprite_imageAtlas[ptr->sprite.imageIndex];
  ptr->frameCounter = 0;
  enemy_addEnemy(ptr);
}

static void
enemy_addNew(void)
{
  enemy_config_t* config = &enemy_configs[enemy_configIndex];

  do {
    int y = enemy_yStarts[game_cameraY+enemy_y[enemy_yIndex]];//-config->height;
    if (!config->onGround) {    
      y -= TILE_HEIGHT;
    }
    
    enemy_yIndex++;
    if (enemy_yIndex >= ENEMY_MAX_Y) {
      enemy_yIndex = 0;
    }

    enemy_t* ptr = enemy_activeList;
    int lineBusy = 0;
    while (ptr != 0) {
      if (ptr->sprite.y == y) {
	lineBusy = 1;
      }
      ptr = ptr->next;
    }
    if (!lineBusy) {
      enemy_add(config->x, y, config->onGround, config->anim);
      break;
    }
  } while(1);

  enemy_configIndex++;
  if (enemy_configIndex >= ENEMY_MAX_CONFIGS) {
    enemy_configIndex = 0;
  }

}

void
enemy_ctor(void)
{
  for (int i = 0; i < WORLD_HEIGHT; i++) {
    enemy_yStarts[i] = ((i / (TILE_HEIGHT*6))*(TILE_HEIGHT*6))+(TILE_HEIGHT*1)+((TILE_HEIGHT*6)*0)-PLAYER_HEIGHT;
    // enemy_yStarts[i] = ((i / (TILE_HEIGHT*6))*(TILE_HEIGHT*6))+(TILE_HEIGHT*1)+((TILE_HEIGHT*6)*0);
  }
}   


void
enemy_init(void)
{
  enemy_configIndex = 0;
  enemy_yIndex = 0;
  enemy_activeList = 0;
  enemy_freeList = &enemy_buffer[0];
  enemy_freeList->prev = 0;
  enemy_t* ptr = enemy_freeList;
  for (int i = 1; i < ENEMY_MAX_ENEMIES; i++) {
      ptr->next = &enemy_buffer[i];
      ptr->next->prev = ptr;
      ptr = ptr->next;
  }

  enemy_addNew();
  enemy_addNew();
  enemy_addNew();
}


void
enemy_saveBackground(frame_buffer_t fb)
{
  enemy_t* ptr = enemy_activeList;

  while (ptr != 0) {
    sprite_save(fb, &ptr->sprite);
    ptr->sprite.save = ptr->sprite.save == &ptr->saves[0] ? &ptr->saves[1] : &ptr->saves[0];
    ptr = ptr->next;
  }
}


void
enemy_restoreBackground(void)
{
  enemy_t* ptr = enemy_activeList;

  while (ptr != 0) {
    sprite_restore(ptr->sprite.save);
    ptr = ptr->next;
  }
}


void
enemy_render(frame_buffer_t fb)
{
  enemy_t* ptr = enemy_activeList;
  int count = 0;

  while (ptr != 0) {
    if (ptr->state != ENEMY_DEAD) {
      sprite_render(fb, ptr->sprite);
    }
    ptr = ptr->next;
    count++;
  }
}

static int
enemy_aabb(sprite_t* p, enemy_t* enemy)
{


  if ((p->x+(ENEMY_COLLISION_FUZZY)) < (enemy->sprite.x+(ENEMY_COLLISION_FUZZY)) + enemy->width &&
      (p->x+(ENEMY_COLLISION_FUZZY)) + PLAYER_WIDTH-(ENEMY_COLLISION_FUZZY) > (enemy->sprite.x+(ENEMY_COLLISION_FUZZY)) &&
      p->y < enemy->sprite.y + enemy->sprite.image->h &&
      PLAYER_HEIGHT + p->y > enemy->sprite.y) {
    return 1;
  }
  return 0;
}

#if 0
int
enemy_collision(sprite_t* p)
{
  enemy_t* ptr = enemy_activeList;

  while (ptr != 0) {
    if (enemy_aabb(p, ptr)) {
      return 1;
    }
    ptr = ptr->next;
  }

  return 0;
}
#endif


void
enemy_update(sprite_t* p)
{
  int removedCount = 0;
  enemy_t* ptr = enemy_activeList;

  while (ptr != 0) {
    int newX =  ptr->sprite.x+ptr->velocity.x;
    if (newX > SCREEN_WIDTH) {
      newX = -ptr->width;
    }
    if (newX < -32) {
      newX = SCREEN_WIDTH;
    }

    int x;
    if (ptr->velocity.x > 0) {
      x = newX + PLAYER_WIDTH - (PLAYER_FUZZY_WIDTH/2);
    } else {
      x = newX + (PLAYER_FUZZY_WIDTH/2);
    }
    int y = ptr->sprite.y + ptr->sprite.image->h;
    if (ptr->onGround && x >= 0 && x < SCREEN_WIDTH) {
      if (BACKGROUND_TILE(x, y) == TILE_SKY) {
	newX = ptr->sprite.x;
	ptr->velocity.x = -ptr->velocity.x;
	if (ptr->velocity.x > 0) {
	  ptr->animId = ptr->animId == ENEMY_ANIM_LEFT_RUN ? ENEMY_ANIM_RIGHT_RUN : ENEMY_ANIM_RIGHT_SKATE;
	  ptr->anim = &enemy_animations[ptr->animId];
	} else {
	  ptr->animId = ptr->animId == ENEMY_ANIM_RIGHT_RUN ? ENEMY_ANIM_LEFT_RUN : ENEMY_ANIM_LEFT_SKATE;
	  ptr->anim = &enemy_animations[ptr->animId];
	}
	ptr->sprite.imageIndex = ptr->anim->animation.start;
	ptr->sprite.image = &sprite_imageAtlas[ptr->sprite.imageIndex];
      }
    }

    ptr->sprite.x = newX;

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

    if (ptr->state != ENEMY_DEAD && enemy_aabb(p, ptr)) {
      //ptr->state = ENEMY_DEAD;
      // ptr->deadRenderCount = 0;
      player_freeFall();
    }

    enemy_t* save = ptr;
    ptr = ptr->next;

    int remove = 0;

    if (save->state == ENEMY_DEAD && (save->deadRenderCount++ > 2)) {
      remove = 1;
    } else  if (game_scrollCount == 0) {
      if ((save->sprite.y-game_cameraY) > SCREEN_HEIGHT) {
	remove = 1;
      }
    }

    if (remove) {
      enemy_remove(save);
      enemy_addFree(save);
      removedCount++;
    }

  }

  while (removedCount--) {
    enemy_addNew();
  }
}
