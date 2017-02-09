#include "game.h"

#define MAX_ENEMIES 10
#define MAX_Y_OFFSETS 5
#define COLLISION_FUZZY 16

typedef enum {
  ENEMY_ALIVE,
  ENEMY_DEAD
} enemy_state_t;


static uint16_t enemy_yStarts[WORLD_HEIGHT];

typedef enum {
  ENEMY_ANIM_RIGHT_RUN = 0,
  ENEMY_ANIM_LEFT_RUN,
  ENEMY_ANIM_RIGHT_SKATE,
  ENEMY_ANIM_LEFT_SKATE
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
  }
};

static enemy_t* enemies;
static enemy_t* freeEnemies;
static int yOffsetIndex;

static enemy_t enemiesBuffer[MAX_ENEMIES];

static int yOffsets[MAX_Y_OFFSETS] = {
    0,
    97,
    194,
    64,
    0
  };

static enemy_anim_t animationIndexes[MAX_Y_OFFSETS] = {
  ENEMY_ANIM_RIGHT_RUN,
  ENEMY_ANIM_LEFT_SKATE,
  ENEMY_ANIM_RIGHT_SKATE,  
  ENEMY_ANIM_LEFT_RUN,    
  ENEMY_ANIM_RIGHT_RUN,  
};


static enemy_t*
enemy_getFree(void)
{
  enemy_t* entry = freeEnemies;
  freeEnemies = freeEnemies->next;
  freeEnemies->prev = 0;
  return entry;
}

static void
enemy_addFree(enemy_t* ptr)
{
  if (freeEnemies == 0) {
    freeEnemies = ptr;
    ptr->next = 0;
    ptr->prev = 0;
  } else {
    ptr->next = freeEnemies;
    ptr->next->prev = ptr;
    ptr->prev = 0;
    freeEnemies = ptr;
  }
}

static void
enemy_addEnemy(enemy_t* ptr)
{
  if (enemies == 0) {
    enemies = ptr;
    ptr->next = 0;
    ptr->prev = 0;
  } else {
    ptr->next = enemies;
    ptr->next->prev = ptr;
    ptr->prev = 0;
    enemies = ptr;
  }
}


static  void
enemy_remove(enemy_t* ptr)
{
  if (ptr->prev == 0) {
    enemies = ptr->next;
    if (enemies) {
      enemies->prev = 0;
    }
  } else {
    ptr->prev->next = ptr->next;
    if (ptr->next != 0) {
      ptr->next->prev = ptr->prev;
    }
  }
}

static void
enemy_add(int y, int anim)
{
  enemy_t* ptr = enemy_getFree();
  ptr->state = ENEMY_ALIVE;
  ptr->width = 32-(COLLISION_FUZZY);
  ptr->sprite.y = y;
  ptr->sprite.save = &ptr->saves[0];
  ptr->saves[0].blit[0].size = 0;
  ptr->saves[0].blit[1].size = 0;
  ptr->saves[1].blit[0].size = 0;
  ptr->saves[0].blit[1].size = 0;
  if (anim == ENEMY_ANIM_LEFT_RUN || anim == ENEMY_ANIM_LEFT_SKATE) {
    ptr->sprite.x = SCREEN_WIDTH;
    ptr->velocity.x = -1;
  } else {
    ptr->sprite.x = -32;
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
  do {
      int y = enemy_yStarts[game_cameraY+yOffsets[yOffsetIndex]];
      enemy_t* ptr = enemies;
      int lineBusy = 0;
      yOffsetIndex++;
      if (yOffsetIndex >= MAX_Y_OFFSETS) {
	yOffsetIndex = 0;
      }
      while (ptr != 0) {
	if (ptr->sprite.y == y) {
	  lineBusy = 1;
	}
	ptr = ptr->next;
      }
      if (!lineBusy) {
	enemy_add(y, animationIndexes[yOffsetIndex]);      
	break;
      }
  } while(1);
}

void
enemy_ctor(void)
{
  for (int i = 0; i < WORLD_HEIGHT; i++) {
    enemy_yStarts[i] = ((i / (TILE_HEIGHT*6))*(TILE_HEIGHT*6))+(TILE_HEIGHT*1)+((TILE_HEIGHT*6)*0)-PLAYER_HEIGHT;
  }
}   


void
enemy_init(void)
{
  yOffsetIndex = 0;
  enemies = 0;
  freeEnemies = &enemiesBuffer[0];
  freeEnemies->prev = 0;
  enemy_t* ptr = freeEnemies;
  for (int i = 1; i < MAX_ENEMIES; i++) {
      ptr->next = &enemiesBuffer[i];
      ptr->next->prev = ptr;
      ptr = ptr->next;
  }

  //enemy_add(-32, WORLD_HEIGHT-128, ENEMY_ANIM_LEFT_RUN);
  //enemy_add(128, WORLD_HEIGHT-SCREEN_HEIGHT+32, ENEMY_ANIM_RIGHT_RUN);
  //  enemy_add(SCREEN_WIDTH/2, WORLD_HEIGHT-(PLAYER_HEIGHT+PLAYER_BASE_PLATFORM_HEIGHT+((TILE_HEIGHT*6)*2)), ENEMY_ANIM_RIGHT_RUN);
  enemy_addNew();
  enemy_addNew();
  enemy_addNew();
}


void
enemy_saveBackground(frame_buffer_t fb)
{
  enemy_t* ptr = enemies;

  while (ptr != 0) {
    sprite_save(fb, &ptr->sprite);
    ptr->sprite.save = ptr->sprite.save == &ptr->saves[0] ? &ptr->saves[1] : &ptr->saves[0];
    ptr = ptr->next;
  }
}


void
enemy_restoreBackground(void)
{
  enemy_t* ptr = enemies;

  while (ptr != 0) {
    sprite_restore(ptr->sprite.save);
    ptr = ptr->next;
  }
}


void
enemy_render(frame_buffer_t fb)
{
  enemy_t* ptr = enemies;

  while (ptr != 0) {
    if (ptr->state != ENEMY_DEAD) {
      sprite_render(fb, ptr->sprite);
    }
    ptr = ptr->next;
  }
}

static int
enemy_aabb(sprite_t* p, enemy_t* enemy)
{


  if ((p->x+(COLLISION_FUZZY)) < (enemy->sprite.x+(COLLISION_FUZZY)) + enemy->width &&
      (p->x+(COLLISION_FUZZY)) + PLAYER_WIDTH-(COLLISION_FUZZY) > (enemy->sprite.x+(COLLISION_FUZZY)) &&
      p->y < enemy->sprite.y + enemy->sprite.image->h &&
      PLAYER_HEIGHT + p->y > enemy->sprite.y) {
    return 1;
  }
  return 0;
}

int
enemy_collision(sprite_t* p)
{
  enemy_t* ptr = enemies;

  while (ptr != 0) {
    if (enemy_aabb(p, ptr)) {
      return 1;
    }
    ptr = ptr->next;
  }

  return 0;
}



void
enemy_update(sprite_t* p)
{
  int removedCount = 0;
  enemy_t* ptr = enemies;

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
    int y = ptr->sprite.y + PLAYER_HEIGHT;
    if (x >= 0 && x < SCREEN_WIDTH) {
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
