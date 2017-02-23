#ifndef __SOUND_H
#define __SOUND_H

typedef enum {
  SOUND_HEADSMASH = 0,
  SOUND_LAND = 1,
  SOUND_PICKUP,
  SOUND_KILL,
  SOUND_FALLING,
  SOUND_MENU
} sound_t;

#if SFX==1
void
sound_vbl(void);
void
sound_queueSound(sound_t sound);
void
sound_schedule(void);
#else
#define sound_vbl()
#define sound_queueSound(x)
#define sound_schedule()
#endif

#endif
