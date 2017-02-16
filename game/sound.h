#ifndef __SOUND_H
#define __SOUND_H


typedef enum {
  SOUND_HEADSMASH = 0,
  SOUND_LAND
} sound_t;

#ifdef GAME_SFX
void
sound_vbl();
void
sound_queue(sound_t sound);
void
sound_schedule(void);
#else
#define sound_vbl()
#define sound_queue(x)
#define sound_schedule()
#endif
#endif
