#ifndef DISK_H
#define DISK_H

void
disk_loadData(__REG("a0", void* dest), __REG("a1", void* src), __REG("d0", int32_t size));

#endif
