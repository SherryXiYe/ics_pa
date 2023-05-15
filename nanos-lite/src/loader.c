#include "common.h"
#include"fs.h"

#define DEFAULT_ENTRY ((void *)0x4000000)

extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);


uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  // size_t ramdisk_size = get_ramdisk_size();
  // ramdisk_read((void *)DEFAULT_ENTRY, 0, ramdisk_size);
  int fd = fs_open(filename, 0, 0);
  fs_read(fd,DEFAULT_ENTRY,fs_filesz(fd));  //把整个filename文件读入DEFAULT_ENTRY处
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;
}
