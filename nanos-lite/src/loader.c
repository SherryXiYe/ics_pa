// #include "common.h"
#include"fs.h"
#include"memory.h"

#define DEFAULT_ENTRY ((void *)0x8048000)

extern size_t get_ramdisk_size();
extern void ramdisk_read(void *buf, off_t offset, size_t len);


uintptr_t loader(_Protect *as, const char *filename) {
  // TODO();
  // size_t ramdisk_size = get_ramdisk_size();
  // ramdisk_read((void *)DEFAULT_ENTRY, 0, ramdisk_size);
  int fd = fs_open(filename, 0, 0);
  // fs_read(fd,DEFAULT_ENTRY,fs_filesz(fd));  //把整个filename文件读入DEFAULT_ENTRY处
  int size = fs_filesz(fd);
  void* va = DEFAULT_ENTRY;
  void* pa;
  while (size > 0) {
    pa = new_page();      //申请物理页
    _map(as, va, pa);
    fs_read(fd, pa, PGSIZE);
    va += PGSIZE;
    size -= PGSIZE;
  }
  fs_close(fd);
  return (uintptr_t)DEFAULT_ENTRY;

}
