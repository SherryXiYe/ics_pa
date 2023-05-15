#include "fs.h"

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);

// 文件记录表
typedef struct {
  char *name;
  size_t size;
  off_t disk_offset;
  off_t open_offset;  //偏移量属性open_offset,来记录目前文件操作的位置
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_FB, FD_EVENTS, FD_DISPINFO, FD_NORMAL};

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  {"stdin (note that this is not the actual stdin)", 0, 0},
  {"stdout (note that this is not the actual stdout)", 0, 0},
  {"stderr (note that this is not the actual stderr)", 0, 0},
  [FD_FB] = {"/dev/fb", 0, 0},
  [FD_EVENTS] = {"/dev/events", 0, 0},
  [FD_DISPINFO] = {"/proc/dispinfo", 128, 0},
#include "files.h"
};

#define NR_FILES (sizeof(file_table) / sizeof(file_table[0]))

void init_fs() {
  // TODO: initialize the size of /dev/fb
}

// 返回文件描述符fd所描述的文件的大小
size_t fs_filesz(int fd) {
  return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode) {
  for(int i = 0; i < NR_FILES; i++){
    Log("filename: %s",file_table[i].name);
    if(strcmp(file_table[i].name, pathname) == 0)
      return i;
  }
  panic("The file indicated by pathname was not found.\n");
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  assert(fd>=0&&fd<NR_FILES);
  if(fd==FD_STDIN||fd==FD_STDOUT||fd==FD_STDERR){
    Log("Invalid fd.\n");
    return 0;
  }
  ssize_t size = file_table[fd].size;
  if(file_table[fd].open_offset + len > size)
    len = size - file_table[fd].open_offset;
  ramdisk_read(buf, file_table[fd].open_offset + file_table[fd].disk_offset, len);
  file_table[fd].open_offset += len;
  return len;
}

int fs_close(int fd) {
  assert(fd>=0&&fd<NR_FILES);
  return 0;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
  return 0;
}

off_t fs_lseek(int fd, off_t offset, int whence){
  return 0;
}