#ifndef DISK_H
#define DISK_H

#if TRACKLOADER==1
#define DISK_SECTOR_ALIGN __attribute__ ((aligned (512)))
#else
#define DISK_SECTOR_ALIGN
#endif

uint16_t
disk_loadData(void* dest, void* src, int32_t size);

uint16_t
disk_read(void* dest, void* src, int32_t size);

uint16_t
disk_write(void* dest, void* src, int16_t numBlocks);
#endif
