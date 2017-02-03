#ifndef __MUSIC_H
#define __MUSIC_H


//extern void music_play(int32_t moduleIndex);
extern void music_setVolume(__REG("d0", int32_t volume));
extern void music_play(__REG("d0", int32_t moduleIndex));

#endif
