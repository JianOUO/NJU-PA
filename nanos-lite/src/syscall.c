#include <common.h>
#include "syscall.h"

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; // a7, 系统调用寄存器
  a[1] = c->GPR2; // a1, 系统调用第一个参数

  switch (a[0]) {
    case SYS_yield: yield(); c->GPRx = 0; break;
    case SYS_exit: halt(a[1]);
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
