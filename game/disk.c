#include "game.h"

extern uint32_t startCode;

#if TRACKLOADER==1

#if PHOTON_TRACKLOADER==1

extern void
LoadMFMB(__REG("a0", void* dest), __REG("d0", uint32_t startSector), __REG("d1", int16_t nrsecs), __REG("a1", void* lastAddr));

#else

void
td_init(void);

void 
td_selectdisk(__REG("d0", uint32_t diskId));

void
td_motoroff(void);

uint32_t
td_write(__REG("d0", uint16_t firstBlock), __REG("d1", uint16_t numBlocks), __REG("a0", void* source));

uint32_t
td_read(__REG("d0", uint16_t firstBlock), __REG("d1", uint16_t numBlocks), __REG("a0", void* destination));

static uint16_t td_initialised = 0;

static void
td_doInit(void)
{
  if (!td_initialised) {
    td_init();
    td_selectdisk(0x19731973);
    td_initialised = 1;
  }
}

#endif
#endif

void
disk_loadData(void* dest, void* src, int32_t size)
{
#if TRACKLOADER==1
  int32_t startSector = ((((uint32_t)src)-((uint32_t)&startCode))>>9)+2; // +2 for bootblock
  int16_t numSectors = (size+512)>>9;
#if PHOTON_TRACKLOADER==1
  LoadMFMB(dest, startSector, -numSectors, ((char*)dest)+size);
#else

#ifdef DEBUG
  if (size < 512) {
    PANIC("panic: disk_loadData: < 512 read");
  }
#endif

  td_doInit();
  td_read(startSector+(numSectors-1), 1, dest);
  char* d = (char*)dest+((numSectors-1)*512);
  char* s = dest;
  for (int16_t i = 0; i < 512 && ((char*)dest)+size; i++, d++, s++) {
    *d = *s;
  }
  if (numSectors > 1) {
    td_read(startSector, numSectors-1, dest);
  }
  td_motoroff();

#endif

#else

  char* d = (char*)dest;
  char* s = (char*)src;
  for (int32_t i = 0; i < size; i++) {
    *d++ = *s++;
  }

#endif
}

void
disk_write(void* src)
{
#if TRACKLOADER==1
#if PHOTON_TRACKLOADER==0
  td_write(0, 1, src);
  td_motoroff();
#endif
#endif
  USE(src);
}
