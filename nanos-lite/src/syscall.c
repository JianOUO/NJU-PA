#include <common.h>
#include "syscall.h"

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
        c->GPRx = -1;
      }
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] write(%d, %p, %u) = %d", fd, buf, count, c->GPRx);
      #endif
      break;
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
