#include "game.h"

__section(bss_c) level_t level;

__NOLOAD level_t level_level1 =
  {
#include "out/background_1-map.c"
    .item_spriteIds = {
#include "out/item_1-sprites.c"
    },
    .spriteBitplanes = {
#include "out/sprite_1.h"
    },
    .spriteMask = {
#include "out/sprite_1-mask.h"
    },
    .fadeIn = {
#include "out/fade_in_1.h"
    },
    .clouds = 0
  };

__NOLOAD level_t level_level2 =
  {
#include "out/background_2-map.c"
    .item_spriteIds = {
#include "out/item_2-sprites.c"
    },
    .spriteBitplanes = {
#include "out/sprite_2.h"
    },
    .spriteMask = {
#include "out/sprite_2-mask.h"
    },
    .fadeIn = {
#include "out/fade_in_2.h"
    },
    .clouds = 1
  };

__NOLOAD level_t level_level3 =
  {
#include "out/background_3-map.c"
    .item_spriteIds = {
#include "out/item_3-sprites.c"
    },
    .spriteBitplanes = {
#include "out/sprite_3.h"
    },
    .spriteMask = {
#include "out/sprite_3-mask.h"
    },
    .fadeIn = {
#include "out/fade_in_3.h"
    },
    .clouds = 0
  };

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
