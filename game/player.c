#include "game.h"

#define JOYSTICK_POS_IDLE      0
#define JOYSTICK_POS_UP        1
#define JOYSTICK_POS_LEFT      7
#define JOYSTICK_POS_RIGHT     3
#define JOYSTICK_POS_DOWN      5
#define JOYSTICK_POS_UPLEFT    8
#define JOYSTICK_POS_UPRIGHT   2
#define JOYSTICK_POS_DOWNLEFT  6
#define JOYSTICK_POS_DOWNRIGHT 4

#define JOYSTICK_IDLE() (hw_joystickPos == 0)
#define JOYSTICK_LEFT() (hw_joystickPos == 7)
#define JOYSTICK_RIGHT() (hw_joystickPos == 3)
#define JOYSTICK_UP() (hw_joystickPos == 1)

#define PLAYER_FUZZY_BOTTOM         0
#define PLAYER_OFFSET_Y             -1
#define PLAYER_VISIBLE_WIDTH        (PLAYER_WIDTH-PLAYER_FUZZY_WIDTH)
#define PLAYER_INITIAL_Y_OFFSET     (PLAYER_HEIGHT+PLAYER_BASE_PLATFORM_HEIGHT)
#define PLAYER_INITIAL_Y            (WORLD_HEIGHT-PLAYER_INITIAL_Y_OFFSET)
#define PLAYER_JUMP_HEIGHT          118 //112
#define PLAYER_SCROLL_THRESHOLD     (96+48)


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


//static 
player_t player;

static void 
player_setAnim(int anim)
{
  if (player.animId != anim) {
    player.animId = anim;
    player.anim = &animations[player.animId];
    player.sprite.imageIndex = player.anim->animation.start;
    player.sprite.image = &sprite_imageAtlas[player.sprite.imageIndex];
    player.frameCounter = 0;
  }
}


void
player_init(void)
{

  player.freeFall = 0;
  player.velocity.x = 0;
  player.velocity.y = 0;
  player.state = PLAYER_STATE_DEFAULT;
  player.flashCounter = 50;
  player.sprite.x = SCREEN_WIDTH-PLAYER_WIDTH-64;
  player.sprite.y = PLAYER_INITIAL_Y;
  player.sprite.imageIndex = 4;
  player.sprite.image = &sprite_imageAtlas[player.sprite.imageIndex];
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


#ifdef NEW_TILE_COLLISION
typedef struct {
  int x;
  int y;
  int tile;
} collision_status_t;
    
static collision_status_t collisionStatus[6];


static void
player_pointCollision(int pointIndex, int x, int y)
{
  collisionStatus[pointIndex].tile =  player_tileOverlaps(x, y);
  collisionStatus[pointIndex].x = x;
  collisionStatus[pointIndex].y = y;
}

static int
player_tileCollision(int x, int y)
{
  player_pointCollision(0, x+PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+y);
  player_pointCollision(1, x+PLAYER_WIDTH-PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+y);
  player_pointCollision(2, x+PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+(y+PLAYER_HEIGHT-PLAYER_FUZZY_BOTTOM));
  player_pointCollision(3, x+PLAYER_WIDTH-PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+(y+PLAYER_HEIGHT-PLAYER_FUZZY_BOTTOM));
  player_pointCollision(4, x+PLAYER_WIDTH-PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+y+(PLAYER_HEIGHT/2)-PLAYER_FUZZY_BOTTOM);
  player_pointCollision(5, x+PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+y+(PLAYER_HEIGHT/2)-PLAYER_FUZZY_BOTTOM);

  for (int i = 0; i < 6; i++) {
    if (TILE_COLLISION(collisionStatus[i].tile)) {
      return 1;
    }
  }
  
  return 0;
}

#else 
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
    if (overlappingTiles[i] != TILE_SKY) {
      return overlappingTiles[i];
    }
  }
  
  return 0;
}
#endif

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


#ifdef NEW_TILE_COLLISION
static int
player_moveX(void)
{
  int newX = player.sprite.x+player.velocity.x;

  
  int collision = player_tileCollision(newX, player.sprite.y);
  if (collision) {
    if (player.velocity.x < 0) {
      int maxX = 0;
      int index = 0;
      for (int i = 0; i < 6; i++) {
	if (TILE_COLLISION(collisionStatus[i].tile) && collisionStatus[i].x > maxX) {
	  maxX = collisionStatus[i].x;
	  index = i;
	}
      }
      newX = ((collisionStatus[index].x>>4)<<4)+TILE_WIDTH-PLAYER_FUZZY_WIDTH+1;
    } else if (player.velocity.x > 0) {
      int minX = 0x7FFFFFFF;
      int index = 0;
      for (int i = 0; i < 6; i++) {
        if (TILE_COLLISION(collisionStatus[i].tile) && collisionStatus[i].x < minX) {
          minX = collisionStatus[i].x;
          index = i;
        }
      }
      newX = ((collisionStatus[index].x>>4)<<4)-(PLAYER_WIDTH)+PLAYER_FUZZY_WIDTH-1;
    }
  }

  if (newX > SCREEN_WIDTH-PLAYER_VISIBLE_WIDTH) {
      newX = SCREEN_WIDTH-PLAYER_VISIBLE_WIDTH;
  } else if (newX < -PLAYER_FUZZY_WIDTH) {
    newX = -PLAYER_FUZZY_WIDTH;
  }

  if (collision) {
    player.velocity.x = 0;
  } else {
    player.velocity.x = newX - player.sprite.x;
  }
  player.sprite.x = newX;
  return collision;
}

static int
player_moveY(void)
{
  int newY = player.sprite.y+player.velocity.y;
  int collision = player_tileCollision(player.sprite.x, newY);
  if (collision) {
    if (player.velocity.y <= 0) {
      int maxY = 0;
      int index = 0;
      for (int i = 0; i < 6; i++) {
	if (TILE_COLLISION(collisionStatus[i].tile) && collisionStatus[i].y > maxY) {
	  maxY = collisionStatus[i].y;
	  index = i;
	}
      }
      newY = ((collisionStatus[index].y>>4)<<4)+TILE_HEIGHT+1;
    } else if (player.velocity.y > 0) {
      int minY = 0x7FFFFFFF;
      int index = 0;
      for (int i = 0; i < 6; i++) {
        if (TILE_COLLISION(collisionStatus[i].tile) && collisionStatus[i].y < minY) {
          minY = collisionStatus[i].y;
          index = i;
        }
      }
      newY = ((collisionStatus[index].y>>4)<<4)-(PLAYER_HEIGHT);
    }
  }

  if (collision) {
    player.velocity.y = 0;
  } else {
    player.velocity.y = newY - player.sprite.y;
  }
  player.sprite.y = newY;
  return collision;
}
#endif


static void
player_respawn(void)
{
  player.state = PLAYER_STATE_DEFAULT;
  player.sprite.y = PLAYER_INITIAL_Y;
  player.velocity.x = 0;
  player.velocity.y = 0;
  if (player.anim->facing == FACING_LEFT) {
    player_setAnim(ANIM_LEFT_STAND);
  } else {
    player_setAnim(ANIM_RIGHT_STAND);
  }      
  player.flashCounter = 50;
}


static int
player_updateAlive(void)
{
  player.velocity.y += PHYSICS_VELOCITY_G;

#ifndef FREEFALL_MODE
  if (player.velocity.y > PHYSICS_TERMINAL_VELOCITY) {
    player.velocity.y = PHYSICS_TERMINAL_VELOCITY;
  }
#endif

  velocity_t intendedVelocity = player.velocity;
  int collision = 0;

#ifdef NEW_TILE_COLLISION
  
  if (player.velocity.x != 0) {
    collision = player_moveX();
  } 
  collision |= player_moveY();
  

#else
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
#endif

  if (collision && intendedVelocity.y > 0 &&  intendedVelocity.y != player.velocity.y /*&& intendedVelocity.x == player.velocity.x*/) {
    player.state = PLAYER_STATE_ONGROUND;
  } else if (PLAYER_HEADSMASH && collision && intendedVelocity.y < 0 &&  intendedVelocity.y != player.velocity.y && intendedVelocity.x == player.velocity.x)  {
    player.velocity.y =0;
    player.state = PLAYER_STATE_HEADCONTACT;
    int x = ((player.sprite.x+((PLAYER_WIDTH-PLAYER_FUZZY_WIDTH)>>1))>>5)<<1;
    int y = (PLAYER_OFFSET_Y+(player.sprite.y-1))>>4;
    //int x = ((player.sprite.x+((PLAYER_WIDTH-PLAYER_FUZZY_WIDTH)/2))/(TILE_WIDTH*2))*2;
    //int y = (PLAYER_OFFSET_Y+(player.sprite.y-1))/TILE_HEIGHT;

#ifdef FIX_TILE_INVALIDATE_BUG
    if (TILE_COLLISION(level.background_tileAddresses[y][x])) {
      level.background_tileAddresses[y][x] = TILE_SKY;
      tile_invalidateTile(x<<4, y<<4, 0);
    }
    if (TILE_COLLISION(level.background_tileAddresses[y][x+1])) {
      level.background_tileAddresses[y][x+1] = TILE_SKY;
      tile_invalidateTile((x+1)<<4, y<<4, 0);
    }
#else
    level.background_tileAddresses[y][x] = TILE_SKY;
    tile_invalidateTile(x<<4, y<<4, 0);
    level.background_tileAddresses[y][x+1] = TILE_SKY;
    tile_invalidateTile((x+1)<<4, y<<4, 0);
#endif


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


  if (player.velocity.y != 0 || player.velocity.x != 0) {
    if (player.frameCounter == player.anim->animation.speed) {
      player.sprite.imageIndex++;
      player.frameCounter = 0;
      if (player.sprite.imageIndex > player.anim->animation.stop) {
	player.sprite.imageIndex = player.anim->animation.start;
      }
    player.sprite.image = &sprite_imageAtlas[player.sprite.imageIndex];
    } else {
      player.frameCounter++;
    }
  }


  if (player.velocity.y == 0 && collision) {
    if (game_cameraY > 0 && player.state == PLAYER_STATE_ONGROUND && game_scrollCount == 0 && (player.sprite.y-game_cameraY) <= (SCREEN_HEIGHT-(PLAYER_SCROLL_THRESHOLD))) {
      game_scrollCount = ((6*16)/SCROLL_PIXELS);
      game_setBackgroundScroll(SCROLL_PIXELS);
    } else if (game_scrollCount == 0 && ((player.sprite.y-game_cameraY) > (SCREEN_HEIGHT - 64))) {
#ifndef FREEFALL_MODE      
      game_scrollCount = ((6*16)/SCROLL_PIXELS)/2;
      game_setBackgroundScroll(-SCROLL_PIXELS*2);
#endif
    }
  } else if (player.velocity.y > 0 && game_scrollCount == 0 && ((player.sprite.y-game_cameraY) > (SCREEN_HEIGHT - 64))) {
#ifndef FREEFALL_MODE      
    game_scrollCount = (((6*16)/SCROLL_PIXELS)/2);
    game_setBackgroundScroll(-SCROLL_PIXELS*2);
#endif
  }


  if (player.freeFall 
#ifdef FREEFALL_MODE
      || (player.velocity.y > 0 && player.sprite.y-game_cameraY > SCREEN_HEIGHT-PLAYER_INITIAL_Y_OFFSET)
#endif
) {
    player.freeFall = 0;
	
    if (game_cameraY % 16 == 0 && game_scrollCount == 0) {
      if (player.state == PLAYER_STATE_ONGROUND && game_cameraY == WORLD_HEIGHT-SCREEN_HEIGHT) {
	player_respawn();
      } else {
	if (player.anim->facing == FACING_LEFT) {
	  player_setAnim(ANIM_LEFT_FALL);
	} else {
	  player_setAnim(ANIM_RIGHT_FALL);
	}
	player.state = PLAYER_STATE_FREEFALL;
	player.velocity.x = 0;
	player.velocity.y = PHYSICS_TERMINAL_VELOCITY;
	game_setBackgroundScroll(-SCROLL_PIXELS*2);
	game_scrollCount = 10000;
      }
    }
  }

  
  return collision;
}


void
player_freeFall(void)
{
  if (player.flashCounter == 0) {
    player.freeFall = 1;
  }
}


static void
player_updateFreeFall(void)
{
  player.sprite.y += player.velocity.y;
  if (player.sprite.y >= PLAYER_INITIAL_Y) {      
    player_respawn();
    game_shakeScreen();
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
