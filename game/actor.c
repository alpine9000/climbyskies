#include "game.h"

#define MAX_BOBS 4

typedef struct {
  int start;
  int end;
} animation_t;  

typedef struct {
  int x;
  int y;
  int animation;
  int bobIndex;
  int deltaX;
  int deltaY;
  int moveCount;
  int nextAnimation;
} actor_t;


static
animation_t animations[] = {
  { 5, 5 },   // 0 - left jumping
  { 4, 4 },   // 1 - left standing
  { 0, 3 },   // 2 - left running
  { 10, 10 }, // 3 - right standing
  { 6, 9 }    // 4 - right running
};

#define NUM_ACTORS 1
static 
actor_t actors[NUM_ACTORS] = {
  {
    .x = SCREEN_WIDTH-32,
    .y = WORLD_HEIGHT-48-(16*3),
    .bobIndex = 4,
    .animation = 1,
    .deltaX = 0,
    .deltaY = 0,
    .moveCount = -1
  }
};

static void actor_stop(void);

void
actor_init(frame_buffer_t fb)
{
  actor_t* a = &actors[0];

  bob_save(fb, a->x, a->y, a->bobIndex);
}

void
actor_render(frame_buffer_t fb)
{
  actor_t* a = &actors[0];

  bob_clear(fb, a->x, a->y, a->bobIndex);

  if (a->moveCount > 0) {
    a->x += a->deltaX;
    a->y += a->deltaY;
    if (a->moveCount % 4 == 0) {
      a->bobIndex++;
      if (a->bobIndex > animations[a->animation].end) {
	a->bobIndex = animations[a->animation].start;
      }
    }
    a->moveCount-=2;
  } else if (a->moveCount == 0) {
    actor_stop();
    a->moveCount--;
  }

  bob_save(fb, a->x, a->y, a->bobIndex);
  bob_render(fb, a->x, a->y, a->bobIndex);
}

void 
actor_jump()
{
  actor_t * a= &actors[0];
  if (a->moveCount == -1) {
    a->deltaX = 0;
    a->nextAnimation = a->animation;
    a->animation = 0;
    a->bobIndex = animations[a->animation].start;
    a->deltaY = -4;
    a->moveCount = 48;
  }
}

void 
actor_left()
{
  actor_t * a= &actors[0];
  if (a->moveCount <= 0) {
    if (a->animation == 0) {
      scrollCount = 1+((6*16)/SCROLL_PIXELS);
    }
    a->deltaX = -2;
    a->deltaY = 0;
    a->animation = 2;
    a->nextAnimation = 1;
    a->bobIndex = animations[a->animation].start;
    a->moveCount = 32;
  }
}

void
actor_right()
{
  actor_t * a= &actors[0];
  if (a->moveCount <= 0) {
    a->deltaX = 2;
    a->deltaY = 0;
    a->animation = 4;
    a->bobIndex = animations[a->animation].start;
    a->nextAnimation = 3;
    a->moveCount = 32;
  }
}


static void 
actor_stop(void)
{
  actor_t * a= &actors[0]; 
  if (a->animation == 0) {
      scrollCount = 1+((6*16)/SCROLL_PIXELS);
  }
  a->animation = a->nextAnimation;
  a->bobIndex = animations[a->animation].start;
}
