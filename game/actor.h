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
  int deltaX;
  int deltaY;
  facing_direction_t facing;
} action_t;


#endif
