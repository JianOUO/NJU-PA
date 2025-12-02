#include <common.h>
#include "syscall.h"

void do_syscall(Context *c) {
  uintptr_t a[4];
  a[0] = c->GPR1; // a7, 系统调用寄存器

  switch (a[0]) {
    case SYS_yield: yield(); c->GPRx = 0; break;
    default: panic("Unhandled syscall ID = %d", a[0]);
  }
}
