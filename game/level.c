#include "game.h"

__section(bss_c) level_t level;

#define __LEVEL_NUMBER 1
#define __LEVEL_HAS_CLOUDS 0
#include "define_level.h"

#define __LEVEL_NUMBER 2
#define __LEVEL_HAS_CLOUDS 1
#include "define_level.h"

#define __LEVEL_NUMBER 3
#define __LEVEL_HAS_CLOUDS 0
#include "define_level.h"

level_t* level_levels[LEVEL_NUM_LEVELS] = {
  &level_level3,
  &level_level2,
  &level_level1,
};

void
level_load(uint16_t index)
{
  if (index >= LEVEL_NUM_LEVELS) {
    game_paused = 1;
  }

  disk_loadData(&level, level_levels[index], sizeof(level_t));
}
