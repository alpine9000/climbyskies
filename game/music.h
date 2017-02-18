#ifndef __MUSIC_H
#define __MUSIC_H

#define MUSIC_MAX_MUSIC_VOLUME 32

extern uint16_t P61_Master;
//extern void music_play(int32_t moduleIndex);
extern void music_setVolume(__REG("d0", int32_t volume));
extern void music_play(__REG("d0", int32_t moduleIndex));

#endif
