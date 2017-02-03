#include "game.h"

extern void
LoadMFMB(__REG("a0", void* dest), __REG("d0", uint32_t startSector), __REG("d1", int16_t nrsecs), __REG("a5", void* lastAddr), __REG("a6", uint32_t customBase));
extern uint32_t startCode;

void 
disk_loadData(__REG("a0", void* dest), __REG("a1", void* src), __REG("d0", int16_t size))
{
#if TRACKLOADER==1
  int startSector = ((((uint32_t)src)-((uint32_t)&startCode))>>9)+2; // +2 for bootblock
  int16_t numSectors = (size+512)>>9;
  LoadMFMB(dest, startSector, -numSectors, ((char*)dest)+size, 0xdff002);
#else
  char* d = (char*)dest;
  char* s = (char*)src;
  for (int i = 0; i < size; i++) {
    *d++ = *s++;
  }
#endif
}
