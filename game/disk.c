#include "game.h"

extern void
LoadMFMB(__REG("a0", void* dest), __REG("d0", uint32_t startSector), __REG("d1", int16_t nrsecs), __REG("a1", void* lastAddr));
extern uint32_t startCode;

void
disk_loadData(void* dest, void* src, int32_t size)
{
#if TRACKLOADER==1
  int32_t startSector = ((((uint32_t)src)-((uint32_t)&startCode))>>9)+2; // +2 for bootblock
  int16_t numSectors = (size+512)>>9;
  LoadMFMB(dest, startSector, -numSectors, ((char*)dest)+size);
#else
  char* d = (char*)dest;
  char* s = (char*)src;
  for (int32_t i = 0; i < size; i++) {
    *d++ = *s++;
  }
#endif
}
