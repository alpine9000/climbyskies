#include "game.h"

__section(bss_c) level_t level;

#define __LEVEL_NUMBER       1
#include "define_level.h"

#define __LEVEL_NUMBER       2
#include "define_level.h"

#define __LEVEL_NUMBER       3
#include "define_level.h"

typedef struct {
  level_t* levelData;
  uint16_t clouds;
  uint16_t moduleIndex;
} level_config_t;

level_config_t level_levels[LEVEL_NUM_LEVELS] = {
  { 
    .levelData = &level_level3,
    .clouds = 0,
    .moduleIndex = 0,
  },
  { 
    .levelData = &level_level2,
    .clouds = 1,
    .moduleIndex = 1,
  },
  { 
    .levelData = &level_level1,
    .clouds = 0,
    .moduleIndex = 0,
  }
};

static uint16_t level_current = 0xFFFF;

void
level_load(uint16_t index)
{
  if (index == level_current) {
    return;
  }

  if (index >= LEVEL_NUM_LEVELS) {
    game_paused = 1;
  }


  message_screenOn("Loading...");

  music_play(level_levels[index].moduleIndex);

  disk_loadData(&level, level_levels[index].levelData, sizeof(level_t));
  level.clouds = level_levels[index].clouds;
  level.moduleIndex = level_levels[index].moduleIndex;

  message_screenOff();

  level_current = index;
}
