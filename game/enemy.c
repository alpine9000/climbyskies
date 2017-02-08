#include "game.h"

#define MAX_ENEMIES 10
#define MAX_Y_OFFSETS 5

typedef struct enemy {
  struct enemy *prev;
  struct enemy *next;
  sprite_t sprite;
  velocity_t velocity;
  sprite_save_t saves[2];
  sprite_animation_t* anim;
  int frameCounter;
  int width;
} enemy_t;

#define CLOUD_ANIM_RIGHT_RUN 0
#define CLOUD_ANIM_LEFT_RUN  1

sprite_animation_t enemy_animations[] = {
  [CLOUD_ANIM_RIGHT_RUN] = {
    .animation = {
    .start = SPRITE_CLIMBER_RUN_RIGHT_1,
    .stop = SPRITE_CLIMBER_RUN_RIGHT_4,
    .speed = 4
    },
    .facing = FACING_RIGHT
  },
  [CLOUD_ANIM_LEFT_RUN] = {
    .animation = {
      .start = SPRITE_CLIMBER_RUN_LEFT_1, 
      .stop = SPRITE_CLIMBER_RUN_LEFT_4,
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
    -32,
    128,
    64,
    32
  };

static int animationIndexes[MAX_Y_OFFSETS] = {
  0,
  0,
  1,
  0,
  1
};

static enemy_t*
enemy_getFree(void)
{
  enemy_t* entry = freeEnemies;
  freeEnemies = freeEnemies->next;
  freeEnemies->prev = 0;
  return entry;
}

//static 
void
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


//static 
 void
enemy_removeInvalid(enemy_t* ptr)
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

void
enemy_add(int x, int y, int anim)
{
  enemy_t* ptr = enemy_getFree();
  ptr->width = 32;
  ptr->sprite.x = x;
  ptr->sprite.y = y;
  ptr->sprite.save = &ptr->saves[0];
  if (anim == CLOUD_ANIM_LEFT_RUN) {
    ptr->velocity.x = -1;
  } else {
    ptr->velocity.x = 1;
  }
  ptr->velocity.y = 0;
  ptr->anim = &enemy_animations[anim];
  ptr->sprite.imageIndex = ptr->anim->animation.start;
  ptr->frameCounter = 0;
  enemy_addEnemy(ptr);
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

  enemy_add(-32, WORLD_HEIGHT-128, CLOUD_ANIM_LEFT_RUN);
  enemy_add(128, WORLD_HEIGHT-SCREEN_HEIGHT+32, CLOUD_ANIM_RIGHT_RUN);
  //enemy_add(SCREEN_WIDTH/3, WORLD_HEIGHT-128+64, CLOUD_ANIM_RIGHT_RUN);
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
    sprite_render(fb, ptr->sprite);
    ptr = ptr->next;
  }
}


void
enemy_update(void)
{
  int removedCount = 0;
  enemy_t* ptr = enemies;

  while (ptr != 0) {
    ptr->sprite.x+=ptr->velocity.x;
    if (ptr->sprite.x > SCREEN_WIDTH) {
      ptr->sprite.x = -ptr->width;
    }
    if (ptr->sprite.x < -32) {
      ptr->sprite.x = SCREEN_WIDTH;
    }

    if (ptr->frameCounter == ptr->anim->animation.speed) {
      ptr->sprite.imageIndex++;
      ptr->frameCounter = 0;
      if (ptr->sprite.imageIndex > ptr->anim->animation.stop) {
	ptr->sprite.imageIndex = ptr->anim->animation.start;
      }
    } else {
      ptr->frameCounter++;
    }

    enemy_t* save = ptr;
    ptr = ptr->next;
    if (scrollCount == 0) {
      if ((save->sprite.y-cameraY) > SCREEN_HEIGHT) {
	enemy_removeInvalid(save);
	enemy_addFree(save);
	removedCount++;
      }
    }
  }

  while (removedCount--) {
    enemy_add(-32, cameraY+yOffsets[yOffsetIndex], animationIndexes[yOffsetIndex]);
    yOffsetIndex++;
    if (yOffsetIndex >= MAX_Y_OFFSETS) {
      yOffsetIndex = 0;
    }
  }
}
