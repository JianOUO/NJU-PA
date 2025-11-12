#include <am.h>
#include <stdlib.h>
#include <stdio.h>
#include <assert.h>

#define BLKSZ 512

static int disk_size = 0;
static FILE *fp = NULL;

// 初始化虚拟磁盘设备
// 在 native 平台上，AM 使用普通文件来模拟磁盘,通过文件 I/O 模拟磁盘读写
void __am_disk_init() {
  const char *diskimg = getenv("diskimg");
  if (diskimg) { // 如果环境变量 diskimg 存在，表示需要使用虚拟磁盘
    fp = fopen(diskimg, "r+"); // 打开磁盘文件，r+ 表示可读写
    if (fp) {
      fseek(fp, 0, SEEK_END);
      disk_size = (ftell(fp) + 511) / 512; // 计算磁盘大小（字节数 / 块大小）,向上取整
      rewind(fp); // 将文件指针移到文件开头
    }
  }
}

// 返回磁盘设备的配置信息
void __am_disk_config(AM_DISK_CONFIG_T *cfg) {
  cfg->present = (fp != NULL); // 磁盘是否存在
  cfg->blksz = BLKSZ; // 每个磁盘块（扇区）的大小为 512 字节
  cfg->blkcnt = disk_size; // 磁盘总块数
}

// 查询磁盘是否就绪
void __am_disk_status(AM_DISK_STATUS_T *stat) {
  stat->ready = 1; // 磁盘总是处于就绪状态,因为Native平台使用文件模拟磁盘，文件 I/O 没有机械延迟，不需要等待寻道或旋转
}

// 执行磁盘块的读写操作（Block I/O）
void __am_disk_blkio(AM_DISK_BLKIO_T *io) {
  if (fp) {
    // io->blkno - 要访问的起始块号（从 0 开始）
    // SEEK_SET - 从文件开头计算偏移
    fseek(fp, io->blkno * BLKSZ, SEEK_SET);
    int ret;
    // io->blkcnt * BLKSZ - 每个元素的大小（总字节数）
    if (io->write) ret = fwrite(io->buf, io->blkcnt * BLKSZ, 1, fp);
    else ret = fread(io->buf, io->blkcnt * BLKSZ, 1, fp);
    assert(ret == 1); // 返回值为成功写入的元素数量，期望返回 1
  }
}
