#include "game.h"

#define JOYSTICK_POS_IDLE  0
#define JOYSTICK_POS_UP    1
#define JOYSTICK_POS_LEFT  7
#define JOYSTICK_POS_RIGHT 3
#define JOYSTICK_POS_DOWN  5
#define JOYSTICK_POS_UPLEFT 8
#define JOYSTICK_POS_UPRIGHT 2
#define JOYSTICK_POS_DOWNLEFT 6
#define JOYSTICK_POS_DOWNRIGHT 4

#define PLAYER_HEIGHT               37
#define PLAYER_WIDTH_FUZZY          8
#define PLAYER_WIDTH                32
#define PLAYER_VISIBLE_WIDTH        (PLAYER_WIDTH-PLAYER_WIDTH_FUZZY)
#define PLAYER_BASE_PLATFORM_HEIGHT (TILE_HEIGHT*3)
#define PLAYER_INITIAL_Y_OFFSET     (PLAYER_HEIGHT+PLAYER_BASE_PLATFORM_HEIGHT)
#define PLAYER_INITIAL_Y            (WORLD_HEIGHT-PLAYER_INITIAL_Y_OFFSET)
#define PLAYER_JUMP_HEIGHT          112
#define PLAYER_SCROLL_THRESHOLD     (96+48)


#define JOYSTICK_IDLE() (hw_joystickPos == 0)
#define JOYSTICK_LEFT() (hw_joystickPos == 7)
#define JOYSTICK_RIGHT() (hw_joystickPos == 3)
#define JOYSTICK_UP() (hw_joystickPos == 1)

#define ACTION_LEFT_JUMP         0
#define ACTION_LEFT_STAND        1
#define ACTION_LEFT_RUN          2
#define ACTION_RIGHT_JUMP        3
#define ACTION_RIGHT_STAND       4
#define ACTION_RIGHT_RUN         5
#define ACTION_LEFT_FALL         6
#define ACTION_LEFT_FALL_LEFT    7
#define ACTION_RIGHT_FALL        8
#define ACTION_RIGHT_FALL_RIGHT  9


typedef struct {
  sprite_t sprite;
  int actionId;
  int deltaX;
  int deltaY;
  int jumpStartY;
  int onGround;
  action_t* action;
  sprite_save_t saves[2];
  int flashCounter;
  int freeFalling;
} player_t;


static
action_t actions[] = {
  [ACTION_LEFT_JUMP] = {
    .deltaX = 0,
    .deltaY = -4,
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_LEFT, 
      .stop = SPRITE_CLIMBER_JUMP_LEFT, 
      .speed = 1
    },
    .facing = FACING_LEFT
  },
  [ACTION_RIGHT_JUMP] = {
    .deltaX = 0,
    .deltaY = -4,
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_RIGHT, 
      .stop = SPRITE_CLIMBER_JUMP_RIGHT, 
      .speed = 1
    },
    .facing = FACING_RIGHT
  },
  [ACTION_LEFT_FALL] = {
    .deltaX = 0,
    .deltaY = 4,
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_LEFT, 
      .stop = SPRITE_CLIMBER_JUMP_LEFT, 
      .speed = 1
    },
    .facing = FACING_LEFT
  },
  [ACTION_RIGHT_FALL] = {
    .deltaX = 0,
    .deltaY = 4,
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_RIGHT, 
      .stop = SPRITE_CLIMBER_JUMP_RIGHT, 
      .speed = 1
    },
    .facing = FACING_RIGHT 
  },
  [ACTION_RIGHT_FALL_RIGHT] = {
    .deltaX = 2,
    .deltaY = 4,
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_RIGHT, 
      .stop = SPRITE_CLIMBER_JUMP_RIGHT, 
      .speed = 1
    },
    .facing = FACING_RIGHT
  },
  [ACTION_LEFT_FALL_LEFT] = {
    .deltaX = -2,
    .deltaY = 4,
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_LEFT, 
      .stop = SPRITE_CLIMBER_JUMP_LEFT, 
      .speed = 1
    },
    .facing = FACING_LEFT
  },

  [ACTION_LEFT_STAND] = { 
    .deltaX = 0,
    .deltaY = 0,
    .animation = {
      .start = SPRITE_CLIMBER_STAND_LEFT, 
      .stop = SPRITE_CLIMBER_STAND_LEFT, 
      .speed = 0 
    },
    .facing = FACING_LEFT
  },
  [ACTION_LEFT_RUN] = {
    .deltaX = -2,
    .deltaY = 0,
    .animation = {
      .start = SPRITE_CLIMBER_RUN_LEFT_1, 
      .stop = SPRITE_CLIMBER_RUN_LEFT_4,
      .speed = 4
    },
    .facing = FACING_LEFT
  },
  [ACTION_RIGHT_STAND] = {
    .deltaX = 0,
    .deltaY = 0,
    .animation = {
      .start = SPRITE_CLIMBER_STAND_RIGHT, 
      .stop = SPRITE_CLIMBER_STAND_RIGHT,
      .speed = 0
    },
    .facing = FACING_RIGHT
  },
  [ACTION_RIGHT_RUN] = {
    .deltaX = 2,
    .deltaY = 0,
    .animation = {
      .start = SPRITE_CLIMBER_RUN_RIGHT_1,
      .stop = SPRITE_CLIMBER_RUN_RIGHT_4,
      .speed = 4 
    },
    .facing = FACING_RIGHT
  }
};


static 
player_t player;


void 
player_setAction(int action)
{
  if (player.actionId != action) {
    player.actionId = action;
    player.action = &actions[player.actionId];
    player.deltaX = player.action->deltaX;
    player.deltaY = player.action->deltaY;
    player.sprite.imageIndex = player.action->animation.start;
  }
}


void
player_init(void)
{
  player.freeFalling = 0;
  player.flashCounter = 50;
  player.sprite.x = SCREEN_WIDTH-PLAYER_WIDTH;
  player.sprite.y = PLAYER_INITIAL_Y;
  player.sprite.imageIndex = 4;
  player.actionId = -1;
  player.deltaX = 0;
  player.deltaY = 0;

  player_setAction(ACTION_LEFT_STAND);

  player.saves[0].blit[0].size = 0;
  player.saves[0].blit[1].size = 0;
  player.saves[1].blit[0].size = 0;
  player.saves[1].blit[1].size = 0;
  player.sprite.save = &player.saves[0];
}


static int
player_onGround(void)
{
  int y = ((player.sprite.y+PLAYER_HEIGHT)>>4); // ((player.sprite.y+PLAYER_HEIGHT)/TILE_HEIGHT);
  int x = (player.sprite.x+PLAYER_WIDTH_FUZZY);
  //  if (x >= 0 && background_tileAddresses[y][x/TILE_WIDTH] != 0) {

  //if (x >= 0 && background_tileAddresses[y][x>>4] != 0) {
  if (x >= 0 && BACKGROUND_TILE(x, y) != 0) {
    return 1;
  }

  x = player.sprite.x+PLAYER_WIDTH-PLAYER_WIDTH_FUZZY;
  //if (x < SCREEN_WIDTH && background_tileAddresses[y][x/TILE_WIDTH] != 0) {
  //if (x < SCREEN_WIDTH && background_tileAddresses[y][x>>4] != 0) {
  if (x < SCREEN_WIDTH && BACKGROUND_TILE(x,y) != 0) {
    return 1;
  } 
  return 0;
}


static void
player_updateDuringMove(void)
{
  static int lastJoystickPos = -1;

  if (lastJoystickPos != hw_joystickPos) {

    switch (hw_joystickPos) {
    case JOYSTICK_POS_IDLE:
      switch (player.actionId) {
      case ACTION_LEFT_FALL_LEFT:
	player_setAction(ACTION_LEFT_FALL);
	break;
      case ACTION_RIGHT_FALL_RIGHT:
	player_setAction(ACTION_RIGHT_FALL);
	break;
      case ACTION_LEFT_RUN:
	player_setAction(ACTION_LEFT_STAND);
	break;
      case ACTION_RIGHT_RUN:
	player_setAction(ACTION_RIGHT_STAND);
	break;
      }
      break;
    case JOYSTICK_POS_LEFT:
      if (player.deltaY == 0) {
	player_setAction(ACTION_LEFT_RUN);
      }
      break;
    case JOYSTICK_POS_RIGHT:
      if (player.deltaY == 0) {
	player_setAction(ACTION_RIGHT_RUN);
      }
      break;
    case JOYSTICK_POS_UP:
      if (player.onGround) {
	if (player.action->facing == FACING_LEFT) {
	  player_setAction(ACTION_LEFT_JUMP);
	} else {
	  player_setAction(ACTION_RIGHT_JUMP);
	}
	player.jumpStartY = player.sprite.y;
      }
      break;
    }
  }

  lastJoystickPos = hw_joystickPos;
  int currentActionId = player.actionId;
  
  if (player.deltaY < 0) { // Jumping
    if (player.jumpStartY - player.sprite.y > PLAYER_JUMP_HEIGHT) {
      if (player.action->facing == FACING_LEFT) {
	player_setAction(ACTION_LEFT_FALL);
      } else {
	player_setAction(ACTION_RIGHT_FALL);
      }
    }
  } else if (player.deltaY > 0) { // Falling
    if (player.onGround) {
      if (player.action->facing == FACING_LEFT) {
	player_setAction(ACTION_LEFT_STAND);
      } else {
	player_setAction(ACTION_RIGHT_STAND);
      }
    }
  } else if (!player.onGround) { // Walking off platform
    if (player.action->facing == FACING_LEFT) {
      player_setAction(ACTION_LEFT_FALL);
    } else {
      player_setAction(ACTION_RIGHT_FALL);
    }
  } else { // On a platform
    if (scrollCount == 0 && (player.sprite.y-cameraY) <= (SCREEN_HEIGHT-(PLAYER_SCROLL_THRESHOLD))) {
      scrollCount = ((6*16)/SCROLL_PIXELS);
    } 
  }
  
  if (player.sprite.y == player.jumpStartY) {
    if (scrollCount == 0 && (player.sprite.y-cameraY) <= (SCREEN_HEIGHT-(PLAYER_SCROLL_THRESHOLD))) { 
      scrollCount = ((6*16)/SCROLL_PIXELS);
    } 
  }

  if (player.deltaY > 0 && player.sprite.y-cameraY > SCREEN_HEIGHT-PLAYER_INITIAL_Y_OFFSET) {
    player.freeFalling = 1;
    game_setBackgroundScroll(-SCROLL_PIXELS);
    scrollCount = 1000;
    player.deltaY = SCROLL_PIXELS;
    player.sprite.y = SCREEN_HEIGHT-PLAYER_INITIAL_Y_OFFSET+cameraY-1;
  }
  
  if (currentActionId != player.actionId) {
    lastJoystickPos = -1;
  } 
}


void
player_update(void)
{
  if (player.flashCounter > 0) {
    player.flashCounter--;
  }

  if (!player.freeFalling) {
    player.onGround = player_onGround();
    player_updateDuringMove();
  } 

  if (player.deltaX != 0 || player.deltaY != 0) {
    player.sprite.x += player.deltaX;
    if (player.sprite.x > SCREEN_WIDTH-PLAYER_VISIBLE_WIDTH) {
      player.sprite.x = SCREEN_WIDTH-PLAYER_VISIBLE_WIDTH;
    } else if (player.sprite.x < -PLAYER_WIDTH_FUZZY) {
      player.sprite.x = -PLAYER_WIDTH_FUZZY;
    }
    player.sprite.y += player.deltaY;
    if (frameCount % player.action->animation.speed == 0) {
      player.sprite.imageIndex++;
      if (player.sprite.imageIndex > player.action->animation.stop) {
	player.sprite.imageIndex = player.action->animation.start;
      }
    }
  }

  if (player.freeFalling) {
    if (player.sprite.y >= PLAYER_INITIAL_Y) {
      player.freeFalling = 0;
      player.sprite.y = PLAYER_INITIAL_Y;
      scrollCount = 0;
      game_setBackgroundScroll(SCROLL_PIXELS);
      player.deltaX = 0;
      player.deltaY = 0;
      if (player.action->facing == FACING_LEFT) {
	player_setAction(ACTION_LEFT_STAND);
      } else {
	player_setAction(ACTION_RIGHT_STAND);
      }      
    }
  }   
}


void
player_saveBackground(frame_buffer_t fb)
{
  sprite_save(fb, &player.sprite);
  player.sprite.save = player.sprite.save == &player.saves[0] ? &player.saves[1] : &player.saves[0];
}


void
player_restoreBackground(void)
{
  sprite_restore(player.sprite.save);
}


void
player_render(frame_buffer_t fb)
{
  if (player.flashCounter == 0) {
    sprite_render(fb, player.sprite);
  } else if (player.flashCounter != 50 && player.flashCounter & 0x4) {
    sprite_render(fb, player.sprite);
  }
}
