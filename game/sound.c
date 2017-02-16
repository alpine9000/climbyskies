#include "game.h"

#ifdef GAME_SFX
extern UWORD audio_jump;

static int _sound_queue[] = {
  0, 0
};

void 
sound_playLand(void)
{
  volatile struct AudChannel *aud = &custom->aud[3];
  
  aud->ac_ptr = &audio_jump;
  aud->ac_per = 443;
  aud->ac_vol = 64;
  aud->ac_len = 112/2;
  custom->dmacon = DMAF_AUD3|DMAF_SETCLR;
}

void 
sound_playHeadBang(void)
{
  volatile struct AudChannel *aud = &custom->aud[3];
  
  aud->ac_ptr = &audio_jump;
  aud->ac_per = 200;
  aud->ac_vol = 64;
  aud->ac_len = 112/2;
  custom->dmacon = DMAF_AUD3|DMAF_SETCLR;
}

void
sound_vbl()
{
  static UWORD empty[2] = {0,0};
  
  for (int i = 3; i < 4; i++) {
    volatile struct AudChannel *aud = &custom->aud[i];    
    aud->ac_len = 2;
    //    aud->ac_per = 1;
    aud->ac_ptr = &empty[0];
  }
}

void
sound_schedule(void)
{
  if (_sound_queue[SOUND_HEADSMASH] > 0) {
    if (--_sound_queue[SOUND_HEADSMASH] == 0) {
      sound_playHeadBang();
    }  
  } else if (_sound_queue[SOUND_LAND] > 0) {
    if (--_sound_queue[SOUND_LAND] == 0) {
      sound_playLand();
    }
  }
}

void
sound_queue(sound_t sound)
{
  _sound_queue[sound] = 2;
}
#endif
