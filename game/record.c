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
  level.record->lastKey = 0xffff;
  level.record->joystickPos = 0;
  level.record->joystickButton = 0;
}


void
record_process(void)
{
#ifdef GAME_RECORDING
  if (level.record->state == RECORD_RECORD && 
      (level.record->lastJoystickPos != hw_joystickPos || 
       level.record->lastJoystickButton != hw_joystickButton || 
       level.record->lastKey != game_keyPressed)) {
    if (level.record->index < RECORD_MAX_RECORD) {
      level.record->buffer[level.record->index].joystickPos = hw_joystickPos;
      level.record->buffer[level.record->index].joystickButton = hw_joystickButton;
      level.record->buffer[level.record->index].key = game_keyPressed;
      level.record->buffer[level.record->index].frame = level.record->frame;
      level.record->lastJoystickPos = hw_joystickPos;
      level.record->lastJoystickButton = hw_joystickButton;
      level.record->lastKey = game_keyPressed;
      level.record->index++;
    }
  } else if (level.record->state == RECORD_PLAYBACK) {
    if (level.record->index < RECORD_MAX_RECORD) {
      if (level.record->buffer[level.record->index].frame == level.record->frame) {
	level.record->joystickPos = level.record->buffer[level.record->index].joystickPos;
	level.record->joystickButton = level.record->buffer[level.record->index].joystickButton;
	game_keyPressed = level.record->buffer[level.record->index].key;
	level.record->index++;
      }
      hw_joystickPos = level.record->joystickPos;
      hw_joystickButton = level.record->joystickButton;
    }

  }

  level.record->frame++;
#endif
}

#endif
