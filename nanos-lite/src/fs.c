#include <fs.h>

typedef size_t (*ReadFn) (void *buf, size_t offset, size_t len);
typedef size_t (*WriteFn) (const void *buf, size_t offset, size_t len);

size_t ramdisk_read(void *buf, size_t offset, size_t len);
size_t ramdisk_write(const void *buf, size_t offset, size_t len);
size_t serial_write(const void *buf, size_t offset, size_t len);
size_t events_read(void *buf, size_t offset, size_t len);
size_t dispinfo_read(void *buf, size_t offset, size_t len);
size_t fb_write(const void *buf, size_t offset, size_t len);

typedef struct {
  char *name;
  size_t size;
  size_t disk_offset;
  ReadFn read;
  WriteFn write;
  size_t open_offset;
} Finfo;

enum {FD_STDIN, FD_STDOUT, FD_STDERR, FD_EVENTS, FD_DISPINFO, FD_FB};

size_t invalid_read(void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

size_t invalid_write(const void *buf, size_t offset, size_t len) {
  panic("should not reach here");
  return 0;
}

/* This is the information about all files in disk. */
static Finfo file_table[] __attribute__((used)) = {
  [FD_STDIN]  = {"stdin", 0, 0, invalid_read, invalid_write},
  [FD_STDOUT] = {"stdout", 0, 0, invalid_read, serial_write},
  [FD_STDERR] = {"stderr", 0, 0, invalid_read, serial_write},
  [FD_EVENTS] = {"/dev/events", 0, 0, events_read, invalid_write},
  [FD_DISPINFO] = {"/proc/dispinfo", 0, 0, dispinfo_read, invalid_write},
  [FD_FB] = {"/dev/fb", 0, 0, invalid_read, fb_write},
#include "files.h"
};

const static int count = LENGTH(file_table);

const char *fs_filename(int fd) {
  assert(fd < count);
  return file_table[fd].name;
}

int fs_open(const char *pathname, int flags, int mode) {
  for (int i = 0; i < count; i++) {
    if (strcmp(pathname, file_table[i].name) == 0) {
      file_table[i].open_offset = 0;
      return i;
    }
  }
  return -1; // 文件不存在
  //assert(0);
}

size_t fs_read(int fd, void *buf, size_t len) {
  assert(fd < count); 
  if (file_table[fd].read != NULL) {
    return file_table[fd].read(buf, file_table[fd].open_offset, len);
  }
  if (len + file_table[fd].open_offset > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].open_offset;
  }
  ramdisk_read(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  file_table[fd].open_offset += len;
  return len;
}

size_t fs_write(int fd, const void *buf, size_t len) {
  assert(fd < count);
  if (fd >= FD_STDIN && fd < FD_FB) { // 特殊文件 && 字符设备
    return file_table[fd].write(buf, file_table[fd].open_offset, len);
  }
  if (len + file_table[fd].open_offset > file_table[fd].size) {
    len = file_table[fd].size - file_table[fd].open_offset;
  }
  if (fd == FD_FB) { // 特殊文件 && 块设备
    file_table[fd].write(buf, file_table[fd].open_offset, len);
  } else { // 普通文件
    ramdisk_write(buf, file_table[fd].disk_offset + file_table[fd].open_offset, len);
  }
  file_table[fd].open_offset += len;
  return len;
}

int fs_close(int fd) {
  return 0;
}

size_t fs_lseek(int fd, size_t offset, int whence) {
  assert(fd < count);
  switch (whence) {
    case SEEK_SET: file_table[fd].open_offset = offset; break;
    case SEEK_CUR: file_table[fd].open_offset += offset; break;
    case SEEK_END: file_table[fd].open_offset = file_table[fd].size + offset; break;
    default: assert(0);
  }
  return file_table[fd].open_offset;
}

void init_fs() { 
  // TODO: initialize the size of /dev/fb
  int width = io_read(AM_GPU_CONFIG).width;
  int height = io_read(AM_GPU_CONFIG).height;
  file_table[FD_FB].size = width * height * 4;
}
