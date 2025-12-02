#include <common.h>
#include "syscall.h"

#define CONFIG_STRACE
#define ANSI_FG_BLUE    "\33[1;34m"
#define ANSI_NONE       "\33[0m"
#define STRACE(format, ...) \
  do { \
    printf(ANSI_FG_BLUE format ANSI_NONE "\n", ## __VA_ARGS__); \
  } while (0)

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; // a7, 系统调用寄存器
  a[1] = c->GPR2; // a1, 系统调用第一个参数

  switch (a[0]) {
    case SYS_yield: {
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] SYS_yield: ret 0x0");
      #endif
      yield(); 
      c->GPRx = 0; 
      break;
    }
    case SYS_exit: {
      #ifdef CONFIG_STRACE
      STRACE("[STRACE] SYS_exit: ret 0x%x", a[1]);
      #endif
      halt(a[1]);
    }
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
