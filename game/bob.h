#ifndef __BOB_H
#define __BOB_H


typedef struct {
  gfx_blit_t blit[2];
} bob_save_t;  


void 
bob_render(frame_buffer_t fb, int16_t x, int16_t y, uint16_t b);
void 
bob_save(frame_buffer_t fb, int16_t x, int16_t y, uint16_t b, bob_save_t* save);
void
bob_clear(bob_save_t* save);

typedef enum  {
  BOB_CLIMBER_RUN_LEFT_1 = 0,
  BOB_CLIMBER_RUN_LEFT_2 = 1,
  BOB_CLIMBER_RUN_LEFT_3 = 2,
  BOB_CLIMBER_RUN_LEFT_4 = 3,
  BOB_CLIMBER_STAND_LEFT = 4,
  BOB_CLIMBER_JUMP_LEFT = 5,
  BOB_CLIMBER_RUN_RIGHT_1 = 6,
  BOB_CLIMBER_RUN_RIGHT_2 = 7,
  BOB_CLIMBER_RUN_RIGHT_3 = 8,
  BOB_CLIMBER_RUN_RIGHT_4 = 9,
  BOB_CLIMBER_STAND_RIGHT = 10,
  BOB_CLIMBER_JUMP_RIGHT = 11,
  BOB_CLOUD_1 = 12,
  BOB_CLOUD_2 = 13
} bob_id_t;


#endif
