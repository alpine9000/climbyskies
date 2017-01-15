#ifndef DISK_H
#define DISK_H

void
disk_LoadDiskData(__reg("a0") uint32_t* dest, __reg("a1") uint32_t* src, __reg("d0") uint32_t size);

#endif
