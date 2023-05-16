#include "fs.h"

extern void ramdisk_read(void *buf, off_t offset, size_t len);
extern void ramdisk_write(const void *buf, off_t offset, size_t len);
extern void dispinfo_read(void *buf, off_t offset, size_t len);
extern void fb_write(const void *buf, off_t offset, size_t len);
extern size_t events_read(void *buf, size_t len);

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

extern void getScreen(int* width,int* height);

void init_fs() {
  // TODO: initialize the size of /dev/fb
  int width=0,height=0;
  getScreen(&width,&height);
  file_table[FD_FB].size=width*height*sizeof(uint32_t);  //FD_FB是显存的文件描述符，每个像素4B
  // Log("FD_FB size=%d",file_table[FD_FB].size);
}

// 返回文件描述符fd所描述的文件的大小
size_t fs_filesz(int fd) {
  return file_table[fd].size;
}

int fs_open(const char *pathname, int flags, int mode) {
  for(int i = 0; i < NR_FILES; i++){
    // Log("filename: %s",file_table[i].name);
    if(strcmp(file_table[i].name, pathname) == 0)
      return i;
  }
  panic("The file indicated by pathname was not found.\n");
  return -1;
}

ssize_t fs_read(int fd, void *buf, size_t len) {
  assert(fd>=0&&fd<NR_FILES);
  ssize_t size = file_table[fd].size;
  if(file_table[fd].open_offset + len > size)
    len = size - file_table[fd].open_offset;
  // Log("fd value : %d",fd);
  switch(fd) {
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:
      Log("Invalid fd.\n");
      return 0;
    case FD_FB:
      Log("fd==FD_FB.\n");
      return 0;
    case FD_DISPINFO:
      dispinfo_read(buf, file_table[fd].open_offset, len);
      file_table[fd].open_offset += len;
      return len;
    case FD_EVENTS:
      return events_read(buf, len);
    default:
      ramdisk_read(buf, file_table[fd].open_offset + file_table[fd].disk_offset, len);
      file_table[fd].open_offset += len;
      return len;
  }
}

int fs_close(int fd) {
  assert(fd>=0&&fd<NR_FILES);
  return 0;
}

ssize_t fs_write(int fd, const void *buf, size_t len){
  assert(fd>=0&&fd<NR_FILES);
  ssize_t size = file_table[fd].size;
  switch(fd) {
    case FD_STDIN:
    case FD_STDOUT:
    case FD_STDERR:
      Log("Invalid fd.\n");
      return 0;
    case FD_DISPINFO:
      Log("fd==FD_DISPINFO.\n");
      return 0;
    case FD_FB:
      fb_write(buf, file_table[fd].open_offset, len);
      file_table[fd].open_offset += len;
      return len;
    default:
      if (file_table[fd].open_offset + len > size)
        len = size - file_table[fd].open_offset;
      ramdisk_write(buf, file_table[fd].open_offset + file_table[fd].disk_offset, len);
      file_table[fd].open_offset += len;
      return len;
  }
}

off_t fs_lseek(int fd, off_t offset, int whence){
  assert(fd>=0&&fd<NR_FILES);
  ssize_t size = file_table[fd].size;
  switch (whence) {
    case SEEK_SET:
      if (offset >= 0)
        file_table[fd].open_offset = offset <= size ? offset : size;
      else
        return -1;
      break;
    case SEEK_CUR:
      if (offset >= 0)
        file_table[fd].open_offset = file_table[fd].open_offset + offset <= size ? file_table[fd].open_offset + offset : size;
      else 
        file_table[fd].open_offset = file_table[fd].open_offset + offset >= 0 ? file_table[fd].open_offset + offset : 0;
      break;
    case SEEK_END:
      if (offset <= 0)
        file_table[fd].open_offset = size + offset >= 0 ? size + offset : 0;
      else
        return -1;
      break;
    default:
      panic("Unhandled whence ID.\n");
      return -1;
  }
  return file_table[fd].open_offset;
}