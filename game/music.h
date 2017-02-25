#ifndef __MUSIC_H
#define __MUSIC_H

#define MUSIC_MAX_MUSIC_VOLUME 32

extern uint16_t P61_Target;
extern uint16_t P61_Master;
//extern void music_play(int32_t moduleIndex);
void 
music_setVolume(__REG("d0", int32_t volume));
void 
music_play(__REG("d0", int32_t moduleIndex));
uint16_t
music_toggle_music(void);

#endif
