#include "game.h"

#if SFX==1
extern UWORD sound_land, sound_coin, sound_pop, sound_kill, sound_falling, sound_jetpack;

static void 
sound_playLand(void);
static void
sound_playHeadSmash(void);
static void
sound_playPickup(void);
static void
sound_playKill(void);
static void
sound_playFalling(void);
static void
sound_playJetpack(void);

typedef struct {
  int16_t count;
  int16_t delay;
  void (*play)(void);
} sound_config_t;

static sound_config_t sound_queue[] = {
  [SOUND_HEADSMASH] = {
    .count = 0,
    .delay = 2,
    .play = &sound_playHeadSmash
  },
  [SOUND_LAND] = {
    .count = 0,
    .delay = 2,
    .play = &sound_playLand
  },
  [SOUND_KILL] = {
    .count = 0,
    .delay = 1,
    .play = &sound_playKill
  },
  [SOUND_PICKUP] = {
    .count = 0,
    .delay = 1,
    .play = &sound_playPickup
  },
  [SOUND_FALLING] = {
    .count = 0,
    .delay = 1,
    .play = &sound_playFalling
  },
  [SOUND_MENU] = {
    .count = 0,
    .delay = 0,
    .play = &sound_playHeadSmash
  },
  [SOUND_JETPACK] = {
    .count = 0,
    .delay = 1,
    .play = &sound_playJetpack
  }
  
};

static int16_t sound_next = -1;
static int16_t sound_loop = -1;

static void 
sound_playLand(void)
{
  volatile struct AudChannel *aud = &custom->aud[3];
  
  aud->ac_ptr = &sound_land;
  aud->ac_per = 321;
  aud->ac_vol = 64;
  aud->ac_len = 914/2;
  custom->dmacon = DMAF_AUD3|DMAF_SETCLR;
}


static void 
sound_playKill(void)
{
  volatile struct AudChannel *aud = &custom->aud[3];
  
  aud->ac_ptr = &sound_kill;
  aud->ac_per = 321;
  aud->ac_vol = 64;
  aud->ac_len = 3509/2;
  custom->dmacon = DMAF_AUD3|DMAF_SETCLR;
}


static void 
sound_playHeadSmash(void)
{
  volatile struct AudChannel *aud = &custom->aud[3];

  aud->ac_ptr = &sound_pop;
  aud->ac_per = 321;
  aud->ac_vol = 64;
  aud->ac_len = 224/2;
  custom->dmacon = DMAF_AUD3|DMAF_SETCLR;
}

static void 
sound_playPickup(void)
{
  volatile struct AudChannel *aud = &custom->aud[3];

  custom->dmacon = DMAF_AUD3;
  sound_vbl();
  hw_waitScanLines(4);  

  aud->ac_ptr = &sound_coin;
  aud->ac_per = 321;
  aud->ac_vol = 64;
  aud->ac_len = 3919/2;
  custom->dmacon = DMAF_AUD3|DMAF_SETCLR;
}

static void 
sound_playFalling(void)
{
  volatile struct AudChannel *aud = &custom->aud[3];

  custom->dmacon = DMAF_AUD3;
  sound_vbl();
  hw_waitScanLines(4);    

  aud->ac_ptr = &sound_falling;
  aud->ac_per = 321;
  aud->ac_vol = 64;
  aud->ac_len = 10506/2;
  custom->dmacon = DMAF_AUD3|DMAF_SETCLR;
}

static void 
sound_playJetpack(void)
{
  volatile struct AudChannel *aud = &custom->aud[3];

  custom->dmacon = DMAF_AUD3;
  sound_vbl();
  hw_waitScanLines(4);    

  aud->ac_ptr = &sound_jetpack;
  aud->ac_per = 221;
  aud->ac_vol = 64;
  aud->ac_len = 9975/2;
  custom->dmacon = DMAF_AUD3|DMAF_SETCLR;
}


void
sound_vbl(void)
{
  static UWORD empty[2] = {0,0};
  
  for (int16_t i = 3; i < 4; i++) {
    if (sound_loop == -1) {
      volatile struct AudChannel *aud = &custom->aud[i];    
      aud->ac_len = 2;
      //    aud->ac_per = 1;
      aud->ac_ptr = &empty[0];
    }
  }
}


void
sound_schedule(void)
{
  if (sound_next >= 0) {
    if (--sound_queue[sound_next].count == 0) {
      (*sound_queue[sound_next].play)();
      sound_next = -1;
    }
  }
}

static void
sound_doQueue(sound_t sound) 
{
  if ((int16_t)sound >= sound_next) {
    if (sound_queue[sound].delay == 0) {
      custom->dmacon = DMAF_AUD3;
      sound_vbl();
      hw_waitScanLines(4);
      (*sound_queue[sound].play)();
      sound_next = -1;
    } else {
      sound_queue[sound].count = sound_queue[sound].delay;
      sound_next = sound;
    }
  }
}


void
sound_queueSound(sound_t sound)
{
  if (sound_loop == -1) {
    sound_doQueue(sound);
  }
}

void
sound_loopSound(sound_t sound)
{
  sound_loop = sound;
  sound_doQueue(sound);
}

void
sound_endLoop(void)
{
  if (sound_loop != -1) {
    sound_loop = -1;
    custom->dmacon = DMAF_AUD3;
    sound_vbl();
    hw_waitScanLines(4);  
  }
}
#endif
