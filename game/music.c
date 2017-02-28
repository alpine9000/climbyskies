#include "game.h"

extern uint16_t P61_Master;
extern uint16_t music_song1;

extern void
P61_Init(__REG("a0", void* module));

__EXTERNAL uint16_t P61_Target = MUSIC_MAX_MUSIC_VOLUME;
__section(bss_c) uint32_t music_module[(MAX_P61_SIZE+512)/4];


void 
music_play(__REG("d0", int32_t moduleIndex))
{
  USE(moduleIndex);
  USE(music_module[0]);
  P61_Master = MUSIC_MAX_MUSIC_VOLUME;
  disk_loadData(&music_module, &music_song1, MAX_P61_SIZE);
  P61_Init(&music_module);
}


uint16_t
music_toggle_music(void)
{
  if (P61_Target == MUSIC_MAX_MUSIC_VOLUME) {
    P61_Target = 0;
  } else {
    P61_Target = MUSIC_MAX_MUSIC_VOLUME;
  }

  return P61_Target;
}


uint16_t
music_enabled(void)
{
  return P61_Target;
}
