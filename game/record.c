#include "game.h"

#ifdef GAME_RECORDING

__EXTERNAL record_t* record_ptr = (record_t*)&level.recordData;

record_state_t
record_getState(void)
{
  return level.record->state;
}


void
record_setState(record_state_t state)
{
  level.record->size = sizeof(record_t);
  level.record->frame = 0;
  level.record->state = state;
  level.record->index = 0;
  level.record->lastJoystickPos = 0xff;
  level.record->lastJoystickButton = 0xff;
  level.record->joystickPos = 0;
  level.record->joystickButton = 0;
}
#endif

