#ifndef __ACTOR_H
#define __ACTOR_H

typedef struct {
  int start;
  int stop;
  int speed;
} animation_t;  


typedef enum {
  FACING_LEFT,
  FACING_RIGHT
} facing_direction_t;

typedef struct {
  animation_t animation;
  facing_direction_t facing;
} sprite_animation_t;


#endif
