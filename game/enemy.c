#include "game.h"

#define ENEMY_MAX_ENEMIES     4
#define ENEMY_MAX_CONFIGS     6
#define ENEMY_MAX_Y           4
#define ENEMY_DROP_THRESHOLD  64
#define ENEMY_MAX_BLIT_WIDTH  48
#define ENEMY_MAX_HEIGHT      37
#define ENEMY_COLLISION_FUZZY 6

typedef enum {
  ENEMY_ALIVE,
  ENEMY_DEAD,
  ENEMY_REMOVED
} enemy_state_t;

typedef struct {
  uint8_t fb[(ENEMY_MAX_BLIT_WIDTH/8)*SCREEN_BIT_DEPTH*ENEMY_MAX_HEIGHT];
} enemy_sprite_save_t;

typedef struct enemy {
  struct enemy *prev;
  struct enemy *next;
  sprite_t sprite;
  velocity_t velocity;
  sprite_save_t saves[2];
  enemy_sprite_save_t saveBuffers[2];
  sprite_animation_t* anim;
  enemy_anim_t animId;
  int16_t frameCounter;
  int16_t width;
  int16_t height;
  enemy_state_t state;
  int16_t deadRenderCount;
  int16_t onGround;
  int16_t skyCount;
  uint16_t* tilePtrHi;
  uint16_t* tilePtrLo;
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
  },
  [ENEMY_ANIM_JOYSTICK] = {
    .animation = {
      .start = SPRITE_JOYSTICK_1,
      .stop = SPRITE_JOYSTICK_2,
      .speed = 25
    },
    .facing = FACING_RIGHT
  }
};


static int16_t enemy_count;
static enemy_t* enemy_activeList;
static enemy_t* enemy_freeList;
static int16_t enemy_configIndex;
static int16_t enemy_yIndex;
static uint16_t enemy_yStarts[WORLD_HEIGHT];
static __section(random_c) enemy_t enemy_buffer[ENEMY_MAX_ENEMIES];

typedef struct {
  int16_t x;
  int16_t y;
  int16_t dx;
  int16_t onGround;
  int16_t height;
  enemy_anim_t anim;
} enemy_config_t;

static enemy_config_t enemy_configs[ENEMY_MAX_CONFIGS] = {
  [ENEMY_ANIM_LEFT_SKATE] = {
    .x = SCREEN_WIDTH,
    .y = 0,
    .dx = -1,
    .onGround = 1,
    .height = PLAYER_HEIGHT,
    .anim = ENEMY_ANIM_LEFT_SKATE
  },
  [ENEMY_ANIM_RIGHT_DRAGON] = {
    .x = -32,
    .y = 4,
    .dx = 1,
    .onGround = 0,
    .height = 25,
    .anim = ENEMY_ANIM_RIGHT_DRAGON
  },
  [ENEMY_ANIM_RIGHT_SKATE] = {
    .x = -32,
    .y = 0,
    .dx = 1,
    .onGround = 1,
    .height = PLAYER_HEIGHT,
    .anim = ENEMY_ANIM_RIGHT_SKATE
  },
  [ENEMY_ANIM_LEFT_RUN] = {
    .x = SCREEN_WIDTH,
    .y = 0,
    .dx = -1,
    .onGround = 1,
    .height = ENEMY_RUN_HEIGHT,
    .anim = ENEMY_ANIM_LEFT_RUN
  },
  [ENEMY_ANIM_RIGHT_RUN] = {
    .x = -32,
    .y = 0,
    .dx = 1,
    .onGround = 1,
    .height = ENEMY_RUN_HEIGHT,
    .anim = ENEMY_ANIM_RIGHT_RUN
  },
  [ENEMY_ANIM_JOYSTICK] = {
    .x = 0,
    .y = 0,
    .dx = 0,
    .onGround = 0,
    .height = 32,
    .anim = ENEMY_ANIM_JOYSTICK
  }
};

static enemy_t*
enemy_getFree(void)
{
  enemy_t* entry = enemy_freeList;
  enemy_freeList = enemy_freeList->next;
  if (enemy_freeList) {
    enemy_freeList->prev = 0;
  }
  return entry;
}


static void
enemy_addFree(enemy_t* ptr)
{
  enemy_count--;

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
  enemy_count++;

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


static void
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

uint16_t
enemy_find(uint16_t id)
{
  enemy_t* ptr = enemy_activeList;
  while (ptr != 0) {
    if (ptr->animId == id) {
      return 1;
    }
    ptr = ptr->next;
  }

  return 0;
}

static void
enemy_add(int16_t x, int16_t y, int16_t dx, int16_t height, int16_t onGround, int16_t anim, uint16_t* tilePtrHi, uint16_t* tilePtrLo)
{
  if (enemy_count >= ENEMY_MAX_ENEMIES || y < TILE_HEIGHT*2) {
    return;
  }

  enemy_t* p = enemy_activeList;
  while (p != 0) {
    if (tilePtrHi && p->sprite.y == y && (*tilePtrHi == *p->tilePtrHi)) {
      return;
    }
    p = p->next;
  }
  enemy_t* ptr = enemy_getFree();
  ptr->tilePtrHi = tilePtrHi;
  ptr->tilePtrLo = tilePtrLo;
  ptr->state = ENEMY_ALIVE;
  ptr->width = 32;
  ptr->height = height;
  ptr->onGround = onGround;
  ptr->sprite.y = y;
  ptr->sprite.save = &ptr->saves[0];
  ptr->sprite.saveBuffer = ptr->saveBuffers[0].fb;
  ptr->sprite.saveBufferHeightOffset = ((48/8)*SCREEN_BIT_DEPTH);
  ptr->saves[0].blit[0].size = 0;
  ptr->saves[0].blit[1].size = 0;
  ptr->saves[1].blit[0].size = 0;
  ptr->saves[1].blit[1].size = 0;
  ptr->sprite.x = x;
  ptr->velocity.x = dx;
  ptr->velocity.y = 0;
  ptr->anim = &enemy_animations[anim];
  ptr->animId = anim;
  ptr->sprite.imageIndex = ptr->anim->animation.start;
  ptr->sprite.image = &sprite_imageAtlas[ptr->sprite.imageIndex];
  ptr->frameCounter = 0;
  ptr->skyCount = 0;
  ptr->deadRenderCount = 0;
  enemy_addEnemy(ptr);
}


void
enemy_addMapObject(int16_t id, int16_t x, int16_t y, uint16_t* tilePtrHi, uint16_t* tilePtrLo)
{
  enemy_config_t* config = &enemy_configs[id];
  enemy_add(x, y-config->height+config->y, config->dx, config->height, config->onGround, config->anim, tilePtrHi, tilePtrLo);
}


void
enemy_ctor(void)
{
  for (int16_t i = 0; i < WORLD_HEIGHT; i++) {
    enemy_yStarts[i] = ((i / (TILE_HEIGHT*6))*(TILE_HEIGHT*6))+(TILE_HEIGHT*1)+((TILE_HEIGHT*6)*0);
    // enemy_yStarts[i] = ((i / (TILE_HEIGHT*6))*(TILE_HEIGHT*6))+(TILE_HEIGHT*1)+((TILE_HEIGHT*6)*0);
  }
}   


void
enemy_init(void)
{
  enemy_count = 0;
  enemy_configIndex = 0;
  enemy_yIndex = 0;
  enemy_activeList = 0;
  enemy_freeList = &enemy_buffer[0];
  enemy_freeList->prev = 0;
  enemy_t* ptr = enemy_freeList;
  for (int16_t i = 1; i < ENEMY_MAX_ENEMIES; i++) {
      ptr->next = &enemy_buffer[i];
      ptr->next->prev = ptr;
      ptr = ptr->next;
  }
  ptr->next = 0;
}


void
enemy_saveBackground(frame_buffer_t fb)
{
  enemy_t* ptr = enemy_activeList;

  while (ptr != 0) {
    sprite_save(fb, &ptr->sprite);
    ptr->sprite.save = ptr->sprite.save == &ptr->saves[0] ? &ptr->saves[1] : &ptr->saves[0];
    ptr->sprite.saveBuffer = ptr->sprite.saveBuffer == ptr->saveBuffers[0].fb ? ptr->saveBuffers[1].fb : ptr->saveBuffers[0].fb;
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


static inline int16_t
enemy_aabb(sprite_t* p, enemy_t* enemy)
{
 #ifndef PLAYER_COLLISION_BOX
  int16_t x1 = p->x + ENEMY_COLLISION_FUZZY; // player
  int16_t w1 = PLAYER_WIDTH-(ENEMY_COLLISION_FUZZY*2); // player
  int16_t x2 = enemy->sprite.x + ENEMY_COLLISION_FUZZY; // enemy
  int16_t w2 = enemy->width - (ENEMY_COLLISION_FUZZY*2); // enemy
  int16_t y1 = p->y + ENEMY_COLLISION_FUZZY; // player
  int16_t h1 = PLAYER_HEIGHT - (ENEMY_COLLISION_FUZZY*2); // player
  int16_t y2 = enemy->sprite.y + ENEMY_COLLISION_FUZZY; // enemy
  int16_t h2 = enemy->height - (ENEMY_COLLISION_FUZZY); //enemy
  
  if (x1 < x2 + w2 &&
      x1 + w1 > x2 &&
      y1 < y2 + h2 &&
      h1 + y1 > y2) {
    return 1;
  }
  return 0;
#else
  int16_t ex1 = enemy->sprite.x + ENEMY_COLLISION_FUZZY; // enemy
  int16_t ex2 = ex1 + enemy->width - (ENEMY_COLLISION_FUZZY*2); // enemy
  int16_t ey1 = enemy->sprite.y + ENEMY_COLLISION_FUZZY; // enemy
  int16_t ey2 = ey1 + (enemy->height-ENEMY_COLLISION_FUZZY); // enemy
  
  if (p->collisionBox.x1 < ex2 &&
      p->collisionBox.x2 > ex1 &&
      p->collisionBox.y1 < ey2 &&
      p->collisionBox.y2 > ey1) {
    return 1;
  }
  return 0;
#endif
}


static inline void
enemy_updateEnemy(enemy_t* ptr)
{
  int16_t newX =  ptr->sprite.x+ptr->velocity.x;
  if (newX > SCREEN_WIDTH) {
    newX = -ptr->width;
    if (game_over || game_levelComplete) {
      ptr->state = ENEMY_REMOVED;
    }
  } else if (newX < -32) {
    newX = SCREEN_WIDTH;
    if (game_over || game_levelComplete) {
      ptr->state = ENEMY_REMOVED;
    }
  }
  
  int16_t x;
  
  if ((ptr->sprite.x >= PLAYER_WIDTH  && ptr->sprite.x < (SCREEN_WIDTH-PLAYER_WIDTH))) {
    if (ptr->velocity.x > 0) {
      x = newX + PLAYER_WIDTH - (PLAYER_FUZZY_WIDTH);
    } else {
      x = newX + (PLAYER_FUZZY_WIDTH);
    }
  } else { // stop enemies just displaying when there is no platform for them
    if (ptr->velocity.x > 0) {
      x = newX + PLAYER_WIDTH - 1;
    } else {
      x = newX + 1;
    }
  }
  
  if (ptr->state == ENEMY_DEAD) {
    ptr->velocity.y += PHYSICS_VELOCITY_G;
    if (ptr->velocity.y > PHYSICS_TERMINAL_VELOCITY) {
      ptr->velocity.y = PHYSICS_TERMINAL_VELOCITY;
    }
  }
  
  ptr->sprite.y += ptr->velocity.y;
  int16_t y = ptr->sprite.y + ptr->height;
  if (ptr->state == ENEMY_ALIVE && ptr->onGround && x >= 0 && x < SCREEN_WIDTH) {
    //if (BACKGROUND_TILE(x, y) == TILE_SKY) {
    if (!TILE_COLLISION(BACKGROUND_TILE(x, y))) {
      if (++ptr->skyCount > 1) { // two skies means nothing to stand on
	ptr->state = ENEMY_REMOVED;
      } else {	    
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
    } else {
      ptr->skyCount = 0;
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
  
  if (game_collisions && ptr->state == ENEMY_ALIVE && enemy_aabb(&player.sprite, ptr)) {
    player_freeFall();
  }
}


void
enemy_render(frame_buffer_t fb)
{
  enemy_t* ptr = enemy_activeList;
  while (ptr != 0) {
    if (ptr->state != ENEMY_REMOVED) {
      sprite_render(fb, ptr->sprite);
    }
    enemy_updateEnemy(ptr);
    ptr = ptr->next;
  }
}


int16_t
enemy_headsmash(int16_t x, int16_t y)
{
  int16_t smash = 0;
  enemy_t* ptr = enemy_activeList;

  while (ptr != 0) {
    if (ptr->state == ENEMY_ALIVE && ptr->onGround && ptr->sprite.y+ptr->height == y && ptr->sprite.x <= x && ptr->sprite.x+ptr->width > x) {
      ptr->state = ENEMY_DEAD;
      ptr->velocity.y = PHYSICS_VELOCITY_KILL;
      game_score += 250;
      smash = 1;
      if (ptr->tilePtrHi) {
	*(ptr->tilePtrHi) = 0;
	*(ptr->tilePtrLo) = 0;
      }

    }
    ptr = ptr->next;
  }
  return smash;
}


void
enemy_update(void)
{
  enemy_t* ptr = enemy_activeList;

  while (ptr != 0) { 
    enemy_t* save = ptr;
    //enemy_updateEnemy(ptr);
    ptr = ptr->next;

    if ((save->state == ENEMY_REMOVED && (save->deadRenderCount++ > 2)) ||
	((save->sprite.y-game_cameraY) > SCREEN_HEIGHT+ENEMY_DROP_THRESHOLD) || 
	((save->sprite.y-game_cameraY) < -(ENEMY_DROP_THRESHOLD))) {
      enemy_remove(save);
      enemy_addFree(save);
    }
  }
}

#ifdef DEBUG
int16_t enemy_getCount(void)
{
  return enemy_count;
}
#endif
