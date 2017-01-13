#ifndef __ACTOR_H
#define __ACTOR_H

typedef struct {
  int start;
  int stop;
  int speed;
} animation_t;  

typedef struct {
  animation_t animation;
  int deltaX;
  int deltaY;
  int moveCount;
} action_t;

typedef struct {
  int x;
  int y;
  int lastX;
  int lastY;
  int lastScrollY;
  int action;
  int bobIndex;
  int deltaX;
  int deltaY;
  int moveCount;
} actor_t;

#endif
