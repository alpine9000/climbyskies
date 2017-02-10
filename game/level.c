#include "game.h"

level_t level;

#if defined(__GNUC__)
__section(section noload)
#else
__section(noload)
#endif
level_t level1 = {
#include "out/background-map.c"
#include "out/item-map.c"
};
