#ifndef DISK_H
#define DISK_H

void
disk_loadData(__reg("a0") void* dest, __reg("a1") void* src, __reg("d0") int16_t size);

#endif
