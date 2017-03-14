#ifndef DISK_H
#define DISK_H

#if TRACKLOADER==1
#define DISK_SECTOR_ALIGN __attribute__ ((aligned (512)))
#else
#define DISK_SECTOR_ALIGN
#endif

void
disk_loadData(void* dest, void* src, int32_t size);

void
disk_read(void* dest, int16_t block, int16_t numBlocks);

void
disk_write(void* dest, void* src, int16_t numBlocks);
#endif
