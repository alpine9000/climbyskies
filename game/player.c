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


#define PLAYER_FUZZY_TOP            3
#define PLAYER_FUZZY_BOTTOM         0
#define PLAYER_OFFSET_Y             -1
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
sprite_animation_t player_animations[] = {
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


typedef struct {
  int16_t x;
  int16_t y;
  int32_t tile;
} collision_status_t;
    
static collision_status_t player_collisionStatus[6];

//static 
player_t player;

#ifdef PLAYER_RECORDING

extern player_record_t player_record;

player_record_state_t
player_getRecord(void)
{
  return player_record.state;
}


void
player_setRecord(player_record_state_t state)
{
  player_record.frame = 0;
  player_record.state = state;
  player_record.index = 0;
  player_record.lastJoystickPos = 0xffffffff;
  player_record.joystickPos = 0;
}
#endif


static void 
player_setAnim(int16_t anim)
{
  if (player.animId != anim) {
    player.animId = anim;
    player.anim = &player_animations[player.animId];
    player.sprite.imageIndex = player.anim->animation.start;
    player.sprite.image = &sprite_imageAtlas[player.sprite.imageIndex];
    player.frameCounter = 0;   
#ifdef PLAYER_HARDWARE_SPRITE
    player.hsprite = &sprite_hspriteAtlas[player.sprite.imageIndex];
#endif
  }
}


void
player_init(menu_command_t command)
{
#ifdef PLAYER_RECORDING

  switch (command) {
  case MENU_COMMAND_REPLAY:
    player_setRecord(PLAYER_RECORD_PLAYBACK);
    break;
  case MENU_COMMAND_RECORD:
    player_setRecord(PLAYER_RECORD_RECORD);
    break;
  case MENU_COMMAND_PLAY:
  default:
    player_setRecord(PLAYER_RECORD_IDLE);
    break;
  }
#endif

#ifdef PLAYER_HARDWARE_SPRITE
  for (int16_t i = 0, index = 1; i < 8; i++) {
    copper.sprpt[index] = ((uint32_t)sprite_nullhsprite& 0xffff);
    index += 2;
    copper.sprpt[index] = (uint32_t)sprite_nullhsprite >> 16;
    index += 2;
  }
#endif

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

#ifdef PLAYER_BLIT_SPRITE_OVERDRAW
  player.hspriteCompatible = 1;
#endif
}


static int32_t
player_tileOverlaps(int32_t x, int32_t y)
{
  if (x >= 0 && x < SCREEN_WIDTH && y >= 0) {
    return BACKGROUND_TILE(x, y);
  }
  return 0;
}


static void
player_pointCollision(int16_t pointIndex, int16_t x, int16_t y)
{
  player_collisionStatus[pointIndex].tile =  player_tileOverlaps(x, y);
  player_collisionStatus[pointIndex].x = x;
  player_collisionStatus[pointIndex].y = y;
}


static int
player_tileCollision(int16_t x, int16_t y)
{
  player_pointCollision(0, x+PLAYER_FUZZY_WIDTH, (PLAYER_FUZZY_TOP+PLAYER_OFFSET_Y)+y);
  player_pointCollision(1, x+PLAYER_WIDTH-(PLAYER_FUZZY_WIDTH), (PLAYER_FUZZY_TOP+PLAYER_OFFSET_Y)+y);
  player_pointCollision(2, x+PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+(y+PLAYER_HEIGHT-PLAYER_FUZZY_BOTTOM));
  player_pointCollision(3, x+PLAYER_WIDTH-(PLAYER_FUZZY_WIDTH), PLAYER_OFFSET_Y+(y+PLAYER_HEIGHT-PLAYER_FUZZY_BOTTOM));
  player_pointCollision(4, x+PLAYER_WIDTH-(PLAYER_FUZZY_WIDTH), PLAYER_OFFSET_Y+y+(PLAYER_HEIGHT/2)-PLAYER_FUZZY_BOTTOM);
  player_pointCollision(5, x+PLAYER_FUZZY_WIDTH, PLAYER_OFFSET_Y+y+(PLAYER_HEIGHT/2)-PLAYER_FUZZY_BOTTOM);

  for (int16_t i = 0; i < 6; i++) {
    if (TILE_COLLISION(player_collisionStatus[i].tile)) {
      return 1;
    }
  }
  
  return 0;
}


static void
player_processJoystick(void)
{
#define NOT_UP_THRESHOLD 1
  static uint16_t notUpCount = NOT_UP_THRESHOLD;

#ifdef PLAYER_RECORDING
  if (player_record.state == PLAYER_RECORD_RECORD && player_record.lastJoystickPos != hw_joystickPos) {
    if (player_record.index < PLAYER_MAX_RECORD) {
      player_record.buffer[player_record.index].joystickPos = hw_joystickPos;
      player_record.buffer[player_record.index].frame = player_record.frame;
      player_record.lastJoystickPos = hw_joystickPos;
      player_record.index++;
    }
  } else if (player_record.state == PLAYER_RECORD_PLAYBACK) {
    if (player_record.index < PLAYER_MAX_RECORD) {
      if (player_record.buffer[player_record.index].frame == player_record.frame) {
	player_record.joystickPos = player_record.buffer[player_record.index].joystickPos;
	player_record.index++;
      }
      hw_joystickPos = player_record.joystickPos;
    }
  }

  player_record.frame++;
#endif

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
player_moveX(void)
{
  int16_t newX = player.sprite.x+player.velocity.x;
  int16_t collision = player_tileCollision(newX, player.sprite.y);

  if (collision) {
    if (player.velocity.x < 0) {
      int16_t maxX = 0;
      int16_t index = 0;
      for (int16_t i = 0; i < 6; i++) {
	if (TILE_COLLISION(player_collisionStatus[i].tile) && player_collisionStatus[i].x > maxX) {
	  maxX = player_collisionStatus[i].x;
	  index = i;
	}
      }
      newX = ((player_collisionStatus[index].x>>4)<<4)+TILE_WIDTH-PLAYER_FUZZY_WIDTH+1;
    } else if (player.velocity.x > 0) {
      int16_t minX = 0x7FFF;
      int16_t index = 0;
      for (int16_t i = 0; i < 6; i++) {
        if (TILE_COLLISION(player_collisionStatus[i].tile) && player_collisionStatus[i].x < minX) {
          minX = player_collisionStatus[i].x;
          index = i;
        }
      }
      newX = ((player_collisionStatus[index].x>>4)<<4)-(PLAYER_WIDTH)+PLAYER_FUZZY_WIDTH-1;
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


static int16_t
player_moveY(void)
{
  int16_t newY = player.sprite.y+player.velocity.y;
  int16_t collision = player_tileCollision(player.sprite.x, newY);
  if (collision) {
    if (player.velocity.y <= 0) {
      int16_t maxY = 0;
      int16_t index = 0;
      for (int16_t i = 0; i < 6; i++) {
	if (TILE_COLLISION(player_collisionStatus[i].tile) && player_collisionStatus[i].y > maxY) {
	  maxY = player_collisionStatus[i].y;
	  index = i;
	}
      }
      newY = ((player_collisionStatus[index].y>>4)<<4)+(TILE_HEIGHT-PLAYER_FUZZY_TOP)+1;
    } else if (player.velocity.y > 0) {
      int16_t minY = 0x7FFF;
      int16_t index = 0;
      for (int16_t i = 0; i < 6; i++) {
        if (TILE_COLLISION(player_collisionStatus[i].tile) && player_collisionStatus[i].y < minY) {
          minY = player_collisionStatus[i].y;
          index = i;
        }
      }
      newY = ((player_collisionStatus[index].y>>4)<<4)-(PLAYER_HEIGHT);
    }
  }

  if (collision) {
    player.velocity.y = 0;
  } else {
    player.velocity.y = newY - player.sprite.y;
  }

#ifndef PLAYER_BLIT_SPRITE_OVERDRAW
  if (newY+PLAYER_HEIGHT - game_cameraY < SCREEN_HEIGHT+SPRITE_MAX_HSPRITE_OVERDRAW) {
    player.sprite.y = newY;
  } else {
    player.sprite.y = game_cameraY + SCREEN_HEIGHT + SPRITE_MAX_HSPRITE_OVERDRAW - PLAYER_HEIGHT;
  }
#else
  player.sprite.y = newY;
  player.hspriteCompatible = ((player.sprite.y+PLAYER_HEIGHT)-game_cameraY) < SCREEN_HEIGHT+SPRITE_MAX_HSPRITE_OVERDRAW;
#endif
  return collision;
}


static void
player_respawn(void)
{
  game_loseLife();
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

  if (player.velocity.y > PHYSICS_TERMINAL_VELOCITY) {
    player.velocity.y = PHYSICS_TERMINAL_VELOCITY;
  }

  velocity_t intendedVelocity = player.velocity;
  int16_t collision = 0;

  if (player.velocity.x != 0) {
    collision = player_moveX();
  } 
  collision |= player_moveY();
  
  if (collision && intendedVelocity.y > 0 &&  player.velocity.y == 0 /*intendedVelocity.y != player.velocity.y *//*&& intendedVelocity.x == player.velocity.x*/) {
    if (player.state != PLAYER_STATE_ONGROUND) {
      player.state = PLAYER_STATE_ONGROUND;
    }
  } else if (collision && intendedVelocity.y < 0 &&  intendedVelocity.y != player.velocity.y && intendedVelocity.x == player.velocity.x)  {
    player.velocity.y =0;
    player.state = PLAYER_STATE_HEADCONTACT;

    int16_t x = ((player.sprite.x+((PLAYER_WIDTH)>>1))>>5)<<1;
    int16_t y = (PLAYER_OFFSET_Y+(player.sprite.y-1))>>4;
    //int16_t x = ((player.sprite.x+((PLAYER_WIDTH-PLAYER_FUZZY_WIDTH)/2))/(TILE_WIDTH*2))*2;
    //int16_t y = (PLAYER_OFFSET_Y+(player.sprite.y-1))/TILE_HEIGHT;

    int16_t kill = 0, smash = 0;
    if (TILE_SMASHABLE(level.tileAddresses[y][x])) {
      level.tileAddresses[y][x] = TILE_SKY;
      tile_invalidateTile(x<<4, y<<4, TILE_SKY);
      kill = enemy_headsmash((x<<4)+(TILE_WIDTH/2), y<<4);
      smash = 1;
    }
    if (TILE_SMASHABLE(level.tileAddresses[y][x+1])) {
      level.tileAddresses[y][x+1] = TILE_SKY;
      tile_invalidateTile((x+1)<<4, y<<4, TILE_SKY);
      kill |= enemy_headsmash(((x+1)<<4)+(TILE_WIDTH/2), y<<4);
      smash |= 1;
    }
    if (!smash) {
      sound_queueSound(SOUND_LAND);
    } else {
      sound_queueSound(kill ? SOUND_KILL : SOUND_HEADSMASH);
    }

  } else {   
    player.state = PLAYER_STATE_DEFAULT;
  }

  static volatile int16_t lastY = -1;

  if (player.velocity.y == 0 && player.state == PLAYER_STATE_ONGROUND) {

    if (player.sprite.y != lastY) {
      sound_queueSound(SOUND_LAND);
    }
    
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

  lastY = player.sprite.y;

  if (player.velocity.y != 0 || player.velocity.x != 0) {
    if (player.frameCounter == player.anim->animation.speed) {
      player.sprite.imageIndex++;
      player.frameCounter = 0;
      if (player.sprite.imageIndex > player.anim->animation.stop) {
	player.sprite.imageIndex = player.anim->animation.start;
      }
    player.sprite.image = &sprite_imageAtlas[player.sprite.imageIndex];
#ifdef PLAYER_HARDWARE_SPRITE
    player.hsprite = &sprite_hspriteAtlas[player.sprite.imageIndex];
#endif
    } else {
      player.frameCounter++;
    }
  }

  if (player.velocity.y == 0 && collision) {
    if (game_cameraY > 0 && player.state == PLAYER_STATE_ONGROUND && game_scrollCount == 0 && (player.sprite.y-game_cameraY) <= (SCREEN_HEIGHT-(PLAYER_SCROLL_THRESHOLD))) {
      game_scrollCount = ((6*16)/SCROLL_PIXELS);
      game_setBackgroundScroll(SCROLL_PIXELS);
    } else if (game_scrollCount == 0 && ((player.sprite.y-game_cameraY) > (SCREEN_HEIGHT - 64))) {
      game_scrollCount = ((6*16)/SCROLL_PIXELS)/2;
      game_setBackgroundScroll(-SCROLL_PIXELS*2);
    }
  } else if (player.velocity.y > 0 && game_scrollCount == 0 && ((player.sprite.y-game_cameraY) > (SCREEN_HEIGHT - 64))) {
    game_scrollCount = (((6*16)/SCROLL_PIXELS)/2);
    game_setBackgroundScroll(-SCROLL_PIXELS*2);
  }

  if (player.sprite.y <= TILE_HEIGHT*7 && player.state == PLAYER_STATE_ONGROUND) {
    game_setLevelComplete();
  }
  
  return collision;
}


void
player_freeFall(void)
{  
  if (player.freeFall < 1 && player.flashCounter == 0 && player.state != PLAYER_STATE_FREEFALL) {
    player.freeFall = 1;
    player.state = PLAYER_STATE_FREEFALL;
    sound_queueSound(SOUND_FALLING);
  }
}


static void
player_updateFreeFall(void)
{
  if (player.freeFall) {
    player.velocity.y = PHYSICS_VELOCITY_KILL;
    player.freeFall = 0;
    if (player.anim->facing == FACING_LEFT) {
      player_setAnim(ANIM_LEFT_FALL);
    } else {
      player_setAnim(ANIM_RIGHT_FALL);
    }
    player.velocity.x = 0;
  } 

  player.velocity.y += PHYSICS_VELOCITY_G;

  if (player.velocity.y > PHYSICS_TERMINAL_VELOCITY) {
    player.velocity.y = PHYSICS_TERMINAL_VELOCITY;
  }

  player.sprite.y += player.velocity.y;

  if (!player.freeFall) {
    if ((player.sprite.y-game_cameraY) > (SCREEN_HEIGHT - 128) && (game_cameraY % 16 == 0) &&  game_scrollCount == 0 && player.velocity.y > 0) {
      game_setBackgroundScroll(-SCROLL_PIXELS*2);
      game_scrollCount = 10000;
    }
  }

  if (game_cameraY >= WORLD_HEIGHT-SCREEN_HEIGHT) {
    game_cameraY = WORLD_HEIGHT-SCREEN_HEIGHT;
    game_scrollCount = 0;
  }

  if (player.sprite.y >= PLAYER_INITIAL_Y) {
    player.sprite.y = PLAYER_INITIAL_Y;
    player.velocity.x = 0;
    player.velocity.y = 0;
    if (game_cameraY == WORLD_HEIGHT-SCREEN_HEIGHT) {    
      player_respawn();
      game_shakeScreen();
    }
  }
}


#ifdef PLAYER_COLLISION_BOX
static inline void
player_calculateCollisionBox(void)
{
  player.sprite.collisionBox.x1 = player.sprite.x + PLAYER_FUZZY_WIDTH;
  player.sprite.collisionBox.x2 = player.sprite.collisionBox.x1 + (PLAYER_WIDTH - PLAYER_FUZZY_WIDTH);
  player.sprite.collisionBox.y1 = player.sprite.y + PLAYER_FUZZY_TOP;
  player.sprite.collisionBox.y2 = player.sprite.y + (PLAYER_HEIGHT - PLAYER_FUZZY_WIDTH);
}
#endif


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

#ifdef PLAYER_COLLISION_BOX
  player_calculateCollisionBox();
#endif
}


void
player_saveBackground(frame_buffer_t fb)
{
#ifdef PLAYER_HARDWARE_SPRITE
#ifdef PLAYER_BLIT_SPRITE_OVERDRAW
  if (!player.hspriteCompatible) {
    sprite_save(fb, &player.sprite);
    player.sprite.save = player.sprite.save == &player.saves[0] ? &player.saves[1] : &player.saves[0];
  } else {
    player.sprite.save->blit[0].size = 0;
    player.sprite.save->blit[1].size = 0;
    player.sprite.save = player.sprite.save == &player.saves[0] ? &player.saves[1] : &player.saves[0];
  }
#else
  USE(fb);
#endif
#else
  sprite_save(fb, &player.sprite);
  player.sprite.save = player.sprite.save == &player.saves[0] ? &player.saves[1] : &player.saves[0];
#endif 
}


void
player_restoreBackground(void)
{
#ifdef PLAYER_HARDWARE_SPRITE
#ifdef PLAYER_BLIT_SPRITE_OVERDRAW
  sprite_restore(player.sprite.save);
#endif
#else
  sprite_restore(player.sprite.save);
#endif
}


#ifdef PLAYER_HARDWARE_SPRITE
void
player_hSpriteRender(void)
{
  int16_t y = player.sprite.y-game_cameraY;
  uint16_t vStartLo = y + RASTER_Y_START;
  uint16_t vStopLo = vStartLo + PLAYER_HEIGHT;
  uint16_t vStopHi = ((vStopLo) & 0x100) >> 8;
  uint16_t hStartHi = (player.sprite.x + RASTER_X_START) >> 1;
  uint16_t hStartHi2 = (player.sprite.x + 16 + RASTER_X_START) >> 1;
  
  if (vStopLo >= RASTER_Y_START + SCREEN_HEIGHT) {
    vStopLo =  RASTER_Y_START + SCREEN_HEIGHT;
  }
  
  player.hsprite->hsprite00->vStartLo = vStartLo;
  player.hsprite->hsprite00->hStartHi = hStartHi;
  player.hsprite->hsprite00->vStopLo =  vStopLo;
  player.hsprite->hsprite00->vStopHi =  vStopHi;
  
  player.hsprite->hsprite01->vStartLo = vStartLo;
  player.hsprite->hsprite01->hStartHi = hStartHi;
  player.hsprite->hsprite01->vStopLo =  vStopLo;
  player.hsprite->hsprite01->vStopHi =  vStopHi;
  
  player.hsprite->hsprite10->vStartLo = vStartLo;
  player.hsprite->hsprite10->hStartHi = hStartHi2;
  player.hsprite->hsprite10->vStopLo =  vStopLo;
  player.hsprite->hsprite10->vStopHi =  vStopHi;
  
  player.hsprite->hsprite11->vStartLo = vStartLo;
  player.hsprite->hsprite11->hStartHi = hStartHi2;
  player.hsprite->hsprite11->vStopLo =  vStopLo;
  player.hsprite->hsprite11->vStopHi =  vStopHi;
}
#endif


void
player_render(frame_buffer_t fb)
{
#ifdef PLAYER_HARDWARE_SPRITE
#ifdef PLAYER_BLIT_SPRITE_OVERDRAW
  if (!player.hspriteCompatible) {
    if (player.flashCounter == 0) {
      sprite_render(fb, player.sprite);
    } else if (player.flashCounter != 50 && player.flashCounter & 0x4) {
      sprite_render(fb, player.sprite);
    }
  }
#else
  USE(fb);
#endif
#else
    if (player.flashCounter == 0) {
      sprite_render(fb, player.sprite);
    } else if (player.flashCounter != 50 && player.flashCounter & 0x4) {
      sprite_render(fb, player.sprite);
    }
#endif

}


#ifdef PLAYER_HARDWARE_SPRITE
void
player_updateCopper(void)
{
  int16_t i, index = 1;
  if (
#ifdef PLAYER_BLIT_SPRITE_OVERDRAW
player.hspriteCompatible && 
#endif
    (player.flashCounter == 0 || (player.flashCounter != 50 && player.flashCounter & 0x4))) {
    copper.sprpt[index] = ((uint32_t)player.hsprite->hsprite00 & 0xffff);
    index += 2;
    copper.sprpt[index] = (uint32_t)player.hsprite->hsprite00 >> 16;
    index += 2;
    copper.sprpt[index] = ((uint32_t)player.hsprite->hsprite01 & 0xffff);
    index += 2;
    copper.sprpt[index] = (uint32_t)player.hsprite->hsprite01 >> 16;
    index += 2;
    copper.sprpt[index] = ((uint32_t)player.hsprite->hsprite10 & 0xffff);
    index += 2;
    copper.sprpt[index] = (uint32_t)player.hsprite->hsprite10 >> 16;
    index += 2;
    copper.sprpt[index] = ((uint32_t)player.hsprite->hsprite11 & 0xffff);
    index += 2;
    copper.sprpt[index] = (uint32_t)player.hsprite->hsprite11 >> 16;
    index += 2;
    i = 4;
  } else {
    i = 0;
  }

  for (; i < 8; i++) {
    copper.sprpt[index] = ((uint32_t)sprite_nullhsprite& 0xffff);
    index += 2;
    copper.sprpt[index] = (uint32_t)sprite_nullhsprite >> 16;
    index += 2;
  }
}
#endif
