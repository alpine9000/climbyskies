#include "game.h"

extern uint16_t P61_Master;
extern uint16_t music_song1;

extern void
P61_Init(__REG("a0", void* module));

__EXTERNAL uint16_t P61_Target = 0;

__section(bss_c) uint32_t music_module1[(MAX_P61_SIZE+512)/4];
__section(bss_c) uint32_t music_module2[(MAX_P61_SIZE+512)/4];

static __NOLOAD DISK_SECTOR_ALIGN uint8_t music_level_a[] = {
#include "out/P61.climbyskies_ingame_a.h"
 } ;

static __NOLOAD DISK_SECTOR_ALIGN uint8_t music_level_b[] = {
#include "out/P61.climbyskies_ingame_b.h"
 } ;

typedef struct {
  uint8_t* data;
  uint32_t length;
} music_song_t;

static music_song_t music_songs[] = {
  { music_level_a, sizeof(music_level_a)},
  { music_level_b, sizeof(music_level_b)}
};

static void* music_data_ptr = music_module1;
static uint16_t music_currentModule = 0xFFFF;

void 
music_play(uint16_t moduleIndex)
{
  if (moduleIndex == music_currentModule) {
    return;
  }
  music_currentModule = moduleIndex;
  uint16_t p61_Target = P61_Target;
  disk_loadData(music_data_ptr, music_songs[moduleIndex].data, music_songs[moduleIndex].length);
  P61_Master = 0;
  P61_Init(music_data_ptr);
  P61_Target = p61_Target;
  music_data_ptr = music_data_ptr == music_module1 ? music_module2 : music_module1;
}

void
music_next(void)
{
  uint16_t moduleIndex = music_currentModule+1;
  if (moduleIndex >= (sizeof(music_songs)/sizeof(music_song_t))) {
    moduleIndex = 0;
  }
  music_play(moduleIndex);
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
