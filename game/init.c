#include "game.h"

void
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
  custom->bplcon3 = 0xc00;
  custom->bplcon4 = 0x11;

  hw_interruptsInit();
  
  gfx_init();
}
