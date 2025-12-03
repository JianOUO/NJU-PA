#include <common.h>
#include "syscall.h"
#include <fs.h>

#define CONFIG_STRACE

#ifdef CONFIG_STRACE
#define ANSI_FG_BLUE    "\33[1;34m"
#define ANSI_NONE       "\33[0m"
#define STRACE(format, ...) \
  do { \
    printf(ANSI_FG_BLUE format ANSI_NONE "\n", ## __VA_ARGS__); \
  } while (0)
#endif

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; // a7, 系统调用寄存器
  a[1] = c->GPR2; // a1, 系统调用第一个参数
  a[2] = c->GPR3; // a2, 系统调用第二个参数
  a[3] = c->GPR4; // a3, 系统调用第三个参数
  switch (a[0]) {
    case SYS_yield: {
      yield(); 
      c->GPRx = 0; 
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] yield() = 0x0");
      #endif
      break;
    }
    case SYS_exit: {
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] exit(0x%x)", a[1]);
      #endif
      halt(a[1]);
    }
    case SYS_write: {
      int fd = (int)a[1];
      const char *buf = (const char *)a[2];
      size_t count = (size_t)a[3];
      if (fd == 1 || fd == 2) { // stdout or stderr
        for (size_t i = 0; i < count; i++) {
          putch(*(buf + i));
        }
        c->GPRx = count;
      } else {
        c->GPRx = fs_write(fd, buf, count);
      }
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] write(\"%s\", %p, %u) = %d", fs_filename(fd), buf, count, c->GPRx);
      #endif
      break;
    }
    case SYS_brk: {
      c->GPRx = 0;
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] brk(%p) = %d", a[0], c->GPRx);
      #endif
      break;
    }
    case SYS_open: {
      const char *filename = (const char *)a[1];
      int flags = (int)a[2];
      int mode = (int)a[3];
      c->GPRx = fs_open(filename, flags, mode);
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] open(\"%s\", %d, %d) = %d", filename, flags, mode, c->GPRx);
      #endif
      break;
    }
    case SYS_read: {
      int fd = (int)a[1];
      void *buf = (void *)a[2];
      size_t count = (size_t)a[3];
      c->GPRx = fs_read(fd, buf, count);
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] read(\"%s\", %p, %u) = %d", fs_filename(fd), buf, count, c->GPRx);
      #endif 
      break;
    }
    case SYS_close: {
      int fd = (int)a[1];
      c->GPRx = fs_close(fd);
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] close(\"%s\") = %d", fs_filename(fd), c->GPRx);
      #endif  
      break;
    }
    case SYS_lseek: {
      int fd = (int)a[1];
      size_t offset = (size_t)a[2];
      int whence = (int)a[3];
      c->GPRx = fs_lseek(fd, offset, whence);
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] lseek(\"%s\", %u, %d) = %u", fs_filename(fd), offset, whence, c->GPRx);
      #endif  
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
