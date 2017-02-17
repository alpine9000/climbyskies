#include "game.h"

extern uint16_t P61_Master;
extern uint16_t music_song1;

__section(bss_c) uint32_t music_module[(MAX_P61_SIZE+512)/4];

extern void
P61_Init(__REG("a0", void* module));

__EXTERNAL void 
 music_play(__REG("d0", int32_t moduleIndex))
{
  USE(moduleIndex);
  USE(music_module[0]);
  P61_Master = 32;
  disk_loadData(&music_module, &music_song1, MAX_P61_SIZE);
  P61_Init(&music_module);
}

asm("section .noload\n"
    "\tcnop 0,512\n"
    "\t_music_song1:\n"
    "\tincbin \"assets/P61.climbyskies_ingame\"\n"
    "\tcnop 0,512\n");
