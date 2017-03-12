#ifndef DISK_H
#define DISK_H

#define DISK_SECTOR_ALIGN __attribute__ ((aligned (512)))

void
disk_loadData(void* dest, void* src, int32_t size);

void
disk_write(void* src);

#endif
