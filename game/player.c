#include "game.h"

#define JOYSTICK_POS_IDLE  0
#define JOYSTICK_POS_UP    1
#define JOYSTICK_POS_LEFT  7
#define JOYSTICK_POS_RIGHT 3

    // 812
    // 7 3
    // 654 

#define PLAYER_HEIGHT 48
#define PLAYER_WIDTH  32

#define JOYSTICK_IDLE() (hw_joystickPos == 0)
#define JOYSTICK_LEFT() (hw_joystickPos == 7)
#define JOYSTICK_RIGHT() (hw_joystickPos == 3)
#define JOYSTICK_UP() (hw_joystickPos == 1)

#define ACTION_LEFT_JUMP        0
#define ACTION_LEFT_STAND       1
#define ACTION_LEFT_RUN         2
#define ACTION_RIGHT_STAND      3
#define ACTION_RIGHT_RUN        4
#define ACTION_FALL_FACING_LEFT 5
#define ACTION_FALL_LEFT        6
#define ACTION_FALL_RIGHT       7

static
action_t actions[] = {
  [ACTION_LEFT_JUMP] = {
    .deltaX = 0,
    .deltaY = -4,
    .moveCount = 64/2,
    .animation = { 
      .start = 5, 
      .stop = 5, 
      .speed = 1
    }   
  },
  [ACTION_FALL_FACING_LEFT] = {
    .deltaX = 0,
    .deltaY = 4,
    .moveCount = 64/2,
    .animation = { 
      .start = 5, 
      .stop = 5, 
      .speed = 1
    }   
  },
  [ACTION_FALL_RIGHT] = {
    .deltaX = 2,
    .deltaY = 4,
    .moveCount = 64/2,
    .animation = { 
      .start = 5, 
      .stop = 5, 
      .speed = 1
    }   
  },
  [ACTION_FALL_LEFT] = {
    .deltaX = -2,
    .deltaY = 4,
    .moveCount = 64/2,
    .animation = { 
      .start = 5, 
      .stop = 5, 
      .speed = 1
    }   
  },

  [ACTION_LEFT_STAND] = { 
    .deltaX = 0,
    .deltaY = 0,
    .moveCount = 0,
    .animation = {
      .start = 4, 
      .stop = 4, 
      .speed = 0 
    }
  },
  [ACTION_LEFT_RUN] = {
    .deltaX = -2,
    .deltaY = 0,
    .moveCount = 16,
    .animation = {
      .start = 0, 
      .stop = 3,
      .speed = 2 
    }
  },
  [ACTION_RIGHT_STAND] = {
    .deltaX = 0,
    .deltaY = 0,
    .moveCount = 0,
    .animation = {
      .start = 10, 
      .stop = 10,
      .speed = 0
    }
  },
  [ACTION_RIGHT_RUN] = {
    .deltaX = 2,
    .deltaY = 0,
    .moveCount = 16,
    .animation = {
      .start = 6,
      .stop = 9,
      .speed = 2 
    }
  }
};


static 
actor_t player = {
  .x = SCREEN_WIDTH-PLAYER_WIDTH,
  .y = WORLD_HEIGHT-PLAYER_HEIGHT-(16*3),
  .bobIndex = 4,
  .action = ACTION_LEFT_STAND,
  .deltaX = 0,
  .deltaY = 0,
  .moveCount = -1
};


void 
player_setAction(int action)
{
  action_t* a = &actions[action];
  player.action = action;
  player.deltaX = a->deltaX;
  player.deltaY = a->deltaY;
  player.bobIndex = a->animation.start;
  player.moveCount = a->moveCount;
}


static void 
player_switchAction(int action)
{
  action_t* a = &actions[action];
  player.action = action;
  player.deltaX = a->deltaX;
  player.deltaY = a->deltaY;
  player.bobIndex = a->animation.start;
}

void
player_init(frame_buffer_t fb)
{
  player_setAction(player.action);
  player.lastX = player.x;
  player.lastY = player.y;
  player.lastScrollY = screenScrollY;
  player_saveBackground(fb);
}


void
player_saveBackground(frame_buffer_t fb)
{
  bob_save(fb, player.x, player.y, player.bobIndex);
}

void
player_restoreBackground(frame_buffer_t fb)
{
  bob_clear(fb, player.lastX, player.lastY, player.bobIndex, player.lastScrollY);
}

static int
player_onGround(void)
{
  int y = ((player.y+PLAYER_HEIGHT)/TILE_HEIGHT);
  int x = player.x/TILE_WIDTH;
  return background_tileAddresses[y][x] != 0;
}

void
player_updateDuringMove(void)
{
  switch (hw_joystickPos) {
  case JOYSTICK_POS_IDLE:
    switch (player.action) {
    case ACTION_FALL_LEFT:
      player_switchAction(ACTION_FALL_FACING_LEFT);
      break;
    }
    break;
  case JOYSTICK_POS_LEFT:
    switch (player.action) {
    case ACTION_FALL_FACING_LEFT:
      player_switchAction(ACTION_FALL_LEFT);
      break;
    }
    break;
  }

  if (player.deltaY > 0) {
    if (player_onGround()) {
      player.moveCount = 0;
    }
  }

  if (player.y-cameraY > SCREEN_HEIGHT-48) {
    player.deltaY = 0;
    player.y = SCREEN_HEIGHT-48+cameraY;
  }
}

void
player_updateAfterMove(void)
{
  int onGround = player_onGround();

  if (!onGround) {
    player_setAction(ACTION_FALL_FACING_LEFT);
    return;
  } else {
    if (scrollCount == 0 && (player.y-cameraY) <= (SCREEN_HEIGHT-96-48)) {
      scrollCount = 1+((6*16)/SCROLL_PIXELS);
    } 
  } 
  
  
  switch (hw_joystickPos) {
  case JOYSTICK_POS_LEFT:
    player_setAction(ACTION_LEFT_RUN);
    break;
  case JOYSTICK_POS_RIGHT:
    player_setAction(ACTION_RIGHT_RUN);
    break;
  case JOYSTICK_POS_UP:
    player_setAction(ACTION_LEFT_JUMP);
    break;
  default:
    switch (player.action) {
    case ACTION_FALL_FACING_LEFT:       
    case ACTION_LEFT_JUMP:
    case ACTION_LEFT_STAND:
    case ACTION_LEFT_RUN:
      player_setAction(ACTION_LEFT_STAND);
      break;
    case ACTION_RIGHT_STAND:
    case ACTION_RIGHT_RUN:
      player_setAction(ACTION_RIGHT_STAND);
      break;
    }     
  }
}

void
player_update(void)
{
  player_updateDuringMove();

  if (player.moveCount > 0) {
    player.x += player.deltaX;
    player.y += player.deltaY;
    action_t* action = &actions[player.action];
    if (player.moveCount % action->animation.speed == 0) {
      player.bobIndex++;
      if (player.bobIndex > action->animation.stop) {
	player.bobIndex =  action->animation.start;
      }
    }
    player.moveCount--;
  } else if (player.moveCount == 0) {
    player_updateAfterMove();
  }
}


void
player_render(frame_buffer_t fb)
{
  bob_render(fb, player.x, player.y, player.bobIndex);
  player.lastX = player.x;
  player.lastY = player.y;
  player.lastScrollY = screenScrollY;
}
