#include "game.h"

static int16_t level3_pulseCount;
static int16_t level3_pulse;
static int16_t level3_deltaPulse;

void
level3_init(void)
{
  level3_pulseCount = 0;
  level3_pulse = 14;
  level3_deltaPulse = 1;
}


void
level3_effect(frame_buffer_t fb)
{
  USE(fb);
  if (level3_pulseCount++ == 5) {
    custom->color[14] = (level3_pulse << 8) | 0x56;
    custom->color[29] = (level3_pulse << 4) | 0x703;
    custom->color[1] = level3_pulse | 0x670;
    level3_pulse+=level3_deltaPulse;
    if (level3_pulse > 0xf) {
      level3_pulse = 0xf;
      level3_deltaPulse= -1;
    } else if (level3_pulse < 8) {
      level3_pulse = 8;
      level3_deltaPulse = 1;
    }
    level3_pulseCount = 0;
  }
}
