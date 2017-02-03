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

#define JOYSTICK_IDLE() (hw_joystickPos == 0)
#define JOYSTICK_LEFT() (hw_joystickPos == 7)
#define JOYSTICK_RIGHT() (hw_joystickPos == 3)
#define JOYSTICK_UP() (hw_joystickPos == 1)

#define PLAYER_WIDTH                32
#define PLAYER_HEIGHT               37
#define PLAYER_FUZZY_WIDTH          8
#define PLAYER_FUZZY_BOTTOM         0
#define PLAYER_OFFSET_Y             -1
#define PLAYER_VISIBLE_WIDTH        (PLAYER_WIDTH-PLAYER_FUZZY_WIDTH)
#define PLAYER_BASE_PLATFORM_HEIGHT (TILE_HEIGHT*3)
#define PLAYER_INITIAL_Y_OFFSET     (PLAYER_HEIGHT+PLAYER_BASE_PLATFORM_HEIGHT)
#define PLAYER_INITIAL_Y            (WORLD_HEIGHT-PLAYER_INITIAL_Y_OFFSET)
#define PLAYER_JUMP_HEIGHT          118 //112
#define PLAYER_SCROLL_THRESHOLD     (96+48)

#define PHYSICS_VELOCITY_G     1
#define PHYSICS_VELOCITY_RUN   2
#define PHYSICS_VELOCITY_JUMP  -16

#define ANIM_LEFT_JUMP         0
#define ANIM_LEFT_STAND        1
#define ANIM_LEFT_RUN          2
#define ANIM_RIGHT_JUMP        3
#define ANIM_RIGHT_STAND       4
#define ANIM_RIGHT_RUN         5
#define ANIM_LEFT_FALL         6
#define ANIM_LEFT_FALL_LEFT    7
#define ANIM_RIGHT_FALL        8
#define ANIM_RIGHT_FALL_RIGHT  9

typedef enum {
  PLAYER_STATE_DEFAULT,
  PLAYER_STATE_FREEFALL,
  PLAYER_STATE_ONGROUND,
  PLAYER_STATE_HEADCONTACT
} player_state_t;

typedef struct {
  int x;
  int y;
} velocity_t;

typedef struct {
  sprite_t sprite;
  int animId;
  velocity_t velocity;
  sprite_animation_t* anim;
  sprite_save_t saves[2];
  int flashCounter;
  player_state_t state;
} player_t;


static
sprite_animation_t animations[] = {
  [ANIM_LEFT_JUMP] = {
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_LEFT, 
      .stop = SPRITE_CLIMBER_JUMP_LEFT, 
      .speed = 1
    },
    .facing = FACING_LEFT
  },
  [ANIM_RIGHT_JUMP] = {
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_RIGHT, 
      .stop = SPRITE_CLIMBER_JUMP_RIGHT, 
      .speed = 1
    },
    .facing = FACING_RIGHT
  },
  [ANIM_LEFT_FALL] = {
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_LEFT, 
      .stop = SPRITE_CLIMBER_JUMP_LEFT, 
      .speed = 1
    },
    .facing = FACING_LEFT
  },
  [ANIM_RIGHT_FALL] = {
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_RIGHT, 
      .stop = SPRITE_CLIMBER_JUMP_RIGHT, 
      .speed = 1
    },
    .facing = FACING_RIGHT 
  },
  [ANIM_RIGHT_FALL_RIGHT] = {
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_RIGHT, 
      .stop = SPRITE_CLIMBER_JUMP_RIGHT, 
      .speed = 1
    },
    .facing = FACING_RIGHT
  },
  [ANIM_LEFT_FALL_LEFT] = {
    .animation = { 
      .start = SPRITE_CLIMBER_JUMP_LEFT, 
      .stop = SPRITE_CLIMBER_JUMP_LEFT, 
      .speed = 1
    },
    .facing = FACING_LEFT
  },

  [ANIM_LEFT_STAND] = { 
    .animation = {
      .start = SPRITE_CLIMBER_STAND_LEFT, 
      .stop = SPRITE_CLIMBER_STAND_LEFT, 
      .speed = 1 
    },
    .facing = FACING_LEFT
  },
  [ANIM_LEFT_RUN] = {
    .animation = {
      .start = SPRITE_CLIMBER_RUN_LEFT_1, 
      .stop = SPRITE_CLIMBER_RUN_LEFT_4,
      .speed = 4
    },
    .facing = FACING_LEFT
  },
  [ANIM_RIGHT_STAND] = {

    .animation = {
      .start = SPRITE_CLIMBER_STAND_RIGHT, 
      .stop = SPRITE_CLIMBER_STAND_RIGHT,
      .speed = 1
    },
    .facing = FACING_RIGHT
  },
  [ANIM_RIGHT_RUN] = {
    .animation = {
      .start = SPRITE_CLIMBER_RUN_RIGHT_1,
      .stop = SPRITE_CLIMBER_RUN_RIGHT_4,
      .speed = 4 
    },
    .facing = FACING_RIGHT
  }
};


static player_t player;


static void 
player_setAnim(int anim)
{
  if (player.animId != anim) {
    player.animId = anim;
    player.anim = &animations[player.animId];
    player.sprite.imageIndex = player.anim->animation.start;
  }
}


void
player_init(void)
{
  player.velocity.x = 0;
  player.velocity.y = 0;
  player.state = PLAYER_STATE_DEFAULT;
  player.flashCounter = 50;
  player.sprite.x = SCREEN_WIDTH-PLAYER_WIDTH;
  player.sprite.y = PLAYER_INITIAL_Y;
  player.sprite.imageIndex = 4;
  player.animId = -1;

  player_setAnim(ANIM_LEFT_STAND);

  player.saves[0].blit[0].size = 0;
  player.saves[0].blit[1].size = 0;
  player.saves[1].blit[0].size = 0;
  player.saves[1].blit[1].size = 0;
  player.sprite.save = &player.saves[0];
}


static int
player_tileOverlaps(int x, int y)
{
  if (x >= 0 && x < SCREEN_WIDTH && y >= 0) {
    return BACKGROUND_TILE(x, y);
  }
  return 0;
}

static int overlappingTiles[6];

static int
player_tileCollision(int x, int y)
{
  overlappingTiles[0] = player_tileOverlaps(x+PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+y);
  overlappingTiles[1] = player_tileOverlaps(x+PLAYER_WIDTH-PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+y);
  overlappingTiles[2] = player_tileOverlaps(x+PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+(y+PLAYER_HEIGHT-PLAYER_FUZZY_BOTTOM));
  overlappingTiles[3] = player_tileOverlaps(x+PLAYER_WIDTH-PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+(y+PLAYER_HEIGHT-PLAYER_FUZZY_BOTTOM));
  overlappingTiles[4] = player_tileOverlaps(x+PLAYER_WIDTH-PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+y+(PLAYER_HEIGHT/2)-PLAYER_FUZZY_BOTTOM);
  overlappingTiles[5] = player_tileOverlaps(x+PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+y+(PLAYER_HEIGHT/2)-PLAYER_FUZZY_BOTTOM);

  for (int i = 0; i < 6; i++) {
    if (overlappingTiles[i] != 0) {
      return overlappingTiles[i];
    }
  }
  
  return 0;
}


static void
player_processJoystick(void)
{
#define NOT_UP_THRESHOLD 1
  static unsigned int notUpCount = NOT_UP_THRESHOLD;

  switch (hw_joystickPos) {
  case JOYSTICK_POS_IDLE:
    player.velocity.x = 0;
    if (player.state == PLAYER_STATE_ONGROUND) {
      notUpCount++;
    }
    break;
  case JOYSTICK_POS_LEFT:
    player.velocity.x = -PHYSICS_VELOCITY_RUN;
    if (player.state == PLAYER_STATE_ONGROUND) {
      notUpCount++;
    }
    break;
  case JOYSTICK_POS_RIGHT:
    player.velocity.x = PHYSICS_VELOCITY_RUN;
    if (player.state == PLAYER_STATE_ONGROUND) {
      notUpCount++;
    }
    break;
  case JOYSTICK_POS_UP:
    if (notUpCount > NOT_UP_THRESHOLD && player.velocity.y == 0 && player.state == PLAYER_STATE_ONGROUND) {
      player.velocity.y = PHYSICS_VELOCITY_JUMP;
    } 
    notUpCount = 0;
    break;
  case JOYSTICK_POS_UPRIGHT:
    player.velocity.x =  PHYSICS_VELOCITY_RUN;
    if (notUpCount > NOT_UP_THRESHOLD && player.velocity.y == 0 && player.state == PLAYER_STATE_ONGROUND) {
      player.velocity.y = PHYSICS_VELOCITY_JUMP;
    } 
    notUpCount = 0;
    break;
  case JOYSTICK_POS_UPLEFT:
    player.velocity.x = -PHYSICS_VELOCITY_RUN;
    if (notUpCount > NOT_UP_THRESHOLD &&  player.velocity.y == 0 && player.state == PLAYER_STATE_ONGROUND) {
      player.velocity.y = PHYSICS_VELOCITY_JUMP;
    }
    notUpCount = 0;
    break;
  }


  return;
}

static int
player_updateAlive(void)
{
  player.velocity.y += PHYSICS_VELOCITY_G;

  velocity_t intendedVelocity = player.velocity;
  int collision = 0;

  if (player.velocity.x != 0) {
    int xInc = (player.velocity.x > 0) ? 1 : -1;
    int x = player.sprite.x;
    for (int c = 0; c != abs(player.velocity.x); c++) {      
      if (!player_tileCollision(x+xInc, player.sprite.y)) {
	x += xInc;
      } else {
	collision = 1;
	break;
      }
    }

    if (x > SCREEN_WIDTH-PLAYER_VISIBLE_WIDTH) {
      x = SCREEN_WIDTH-PLAYER_VISIBLE_WIDTH;
    } else if (x < -PLAYER_FUZZY_WIDTH) {
      x = -PLAYER_FUZZY_WIDTH;
    }
    player.velocity.x = x - player.sprite.x;
    player.sprite.x = x;
  }

  if (player.velocity.y != 0) {
    int speedY = player.velocity.y;
    int yInc = (speedY > 0) ? 1 : -1;
    int y = player.sprite.y;
    for (int c = 0; c != abs(speedY); c++) {      
      if (!player_tileCollision(player.sprite.x, y+yInc)) {
	y += yInc;
      } else {
	collision = 1;
	break;
      }
    }
    player.velocity.y = y - player.sprite.y;
    player.sprite.y = y;
  } 

  if (collision && intendedVelocity.y > 0 &&  intendedVelocity.y != player.velocity.y /*&& intendedVelocity.x == player.velocity.x*/) {
    player.state = PLAYER_STATE_ONGROUND;
  } else if (collision && intendedVelocity.y < 0 &&  intendedVelocity.y != player.velocity.y && intendedVelocity.x == player.velocity.x)  {
    player.velocity.y =0;
    player.state = PLAYER_STATE_HEADCONTACT;
    int x = ((player.sprite.x+((PLAYER_WIDTH-PLAYER_FUZZY_WIDTH)>>1))>>5)<<1;
    int y = (PLAYER_OFFSET_Y+(player.sprite.y-1))>>4;
    //int x = ((player.sprite.x+((PLAYER_WIDTH-PLAYER_FUZZY_WIDTH)/2))/(TILE_WIDTH*2))*2;
    //int y = (PLAYER_OFFSET_Y+(player.sprite.y-1))/TILE_HEIGHT;
    backgroundTiles[y][x] = 0;
    backgroundTiles[y][x+1] = 0;
    tile_invalidateTile(x<<4, y<<4, 0);
    tile_invalidateTile((x+1)<<4, y<<4, 0);
    // tile_invalidateTile(x*TILE_WIDTH, y*TILE_HEIGHT, 0);
    // tile_invalidateTile((x+1)*(TILE_WIDTH), y*TILE_HEIGHT, 0);
  } else {   
    player.state = PLAYER_STATE_DEFAULT;
  }

  
  if (player.velocity.y == 0 && player.state == PLAYER_STATE_ONGROUND) {
    if (player.velocity.x < 0) {
      player_setAnim(ANIM_LEFT_RUN);
    } else if (player.velocity.x > 0) {
      player_setAnim(ANIM_RIGHT_RUN);
    } else {
      if (player.anim->facing == FACING_LEFT) {
	player_setAnim(ANIM_LEFT_STAND);
      } else {
	player_setAnim(ANIM_RIGHT_STAND);
      }
    }
  } else if (player.velocity.y > 0) {
    if (player.velocity.x < 0) {
	player_setAnim(ANIM_LEFT_JUMP);
    } else if (player.velocity.x > 0) {
    	player_setAnim(ANIM_RIGHT_JUMP);
    } else {  
      if (player.anim->facing == FACING_LEFT) {
	player_setAnim(ANIM_LEFT_JUMP);
      } else {
	player_setAnim(ANIM_RIGHT_JUMP);
      }
    }
  } else if (player.velocity.y < 0) {
    if (player.velocity.x < 0) {
	player_setAnim(ANIM_LEFT_FALL);
    } else if (player.velocity.x > 0) {
    	player_setAnim(ANIM_RIGHT_FALL);
    } else {  
      if (player.anim->facing == FACING_LEFT) {
	player_setAnim(ANIM_LEFT_FALL);
      } else {
	player_setAnim(ANIM_RIGHT_FALL);
      }
    }
  }

#if 0
  if (player.velocity.y != 0 || player.velocity.x != 0) {
    if (frameCount % player.anim->animation.speed == 0) {
      player.sprite.imageIndex++;
      if (player.sprite.imageIndex > player.anim->animation.stop) {
	player.sprite.imageIndex = player.anim->animation.start;
      }
    }
  }
#endif

  if (player.velocity.y == 0 && collision) {
    if (scrollCount == 0 && (player.sprite.y-cameraY) <= (SCREEN_HEIGHT-(PLAYER_SCROLL_THRESHOLD))) {
      scrollCount = ((6*16)/SCROLL_PIXELS);
      game_setBackgroundScroll(SCROLL_PIXELS);
    } 
  }

  if (player.velocity.y > 0 && player.sprite.y-cameraY > SCREEN_HEIGHT-PLAYER_INITIAL_Y_OFFSET) {
    if (cameraY % 16 == 0) {
      player.state = PLAYER_STATE_FREEFALL;
      player.velocity.x = 0;
      player.velocity.y = (SCROLL_PIXELS*2);
      game_setBackgroundScroll(-SCROLL_PIXELS*2);
      scrollCount = 10000;
      //      player.sprite.y = SCREEN_HEIGHT-PLAYER_INITIAL_Y_OFFSET+cameraY-1;
    }
  }
  
  return collision;
}


static void
player_updateFreeFall(void)
{
  player.sprite.y += player.velocity.y;
  if (player.sprite.y >= PLAYER_INITIAL_Y) {      
    player.state = PLAYER_STATE_DEFAULT;
    player.sprite.y = PLAYER_INITIAL_Y;
    scrollCount = 0;
    game_setBackgroundScroll(SCROLL_PIXELS);
    player.velocity.x = 0;
    player.velocity.y = 0;
    if (player.anim->facing == FACING_LEFT) {
      player_setAnim(ANIM_LEFT_STAND);
    } else {
      player_setAnim(ANIM_RIGHT_STAND);
    }      
    game_shakeScreen();
    player.flashCounter = 50;
    scrollCount = (WORLD_HEIGHT-SCREEN_HEIGHT - cameraY)>>1;
    game_setBackgroundScroll(-2);
  }
}


void
player_update(void)
{
  if (player.flashCounter > 0) {
    player.flashCounter--;
  }

  if (player.state == PLAYER_STATE_FREEFALL) {
    player_updateFreeFall();
  } else {
    player_updateAlive();
    player_processJoystick();
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
