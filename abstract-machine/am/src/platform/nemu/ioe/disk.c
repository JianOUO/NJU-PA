#include <am.h>
#include <nemu.h>

// 磁盘配置查询
void __am_disk_config(AM_DISK_CONFIG_T *cfg) {
  cfg->present = false; // 磁盘设备不存在
}

// 磁盘状态查询
void __am_disk_status(AM_DISK_STATUS_T *stat) {
}

// 执行磁盘块的读写操作（Block I/O）
void __am_disk_blkio(AM_DISK_BLKIO_T *io) {
}
