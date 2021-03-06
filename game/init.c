#include "game.h"

__EXTERNAL void
memory_ctor(void) 
{
#if TRACKLOADER==1
  extern char startBSS;
  extern char endBSS;

  memset(&startBSS, 0, &endBSS-&startBSS);
#endif
}

__EXTERNAL void
init_amiga(void) 
{
  custom->dmacon = 0x7ff;  /* disable all dma */
  custom->intena = 0x7fff; /* disable all interrupts */

  hw_waitVerticalBlank();

  custom->intena = 0x7fff; /* disable all interrupts */
  custom->dmacon = 0x7fff; /* disable all dma */
  custom->intreq = 0x7fff; /* Clear all INT requests */
  custom->intreq = 0x7fff; /* Clear all INT requests */


  /* AGA compatibility stuff */
  custom->fmode = 0;
  custom->bplcon2 = 0x24;
  //custom->bplcon2 = 0x0;
  custom->bplcon3 = 0xc00;
  custom->bplcon4 = 0x11;

  memory_ctor();
  enemy_ctor();
  gfx_ctor();
  sprite_ctor();
  popup_ctor();
  
#ifdef GAME_KEYBOARD_ENABLED
  keyboard_ctor();
#endif
}
