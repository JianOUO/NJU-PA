#include <am.h>
#include <riscv/riscv.h>
#include <klib.h>

void __am_get_cur_as(Context *c);
void __am_switch(Context *c);

static Context* (*user_handler)(Event, Context*) = NULL;

Context* __am_irq_handle(Context *c) {
  __am_get_cur_as(c);
  if (user_handler) {
    Event ev = {0};
    switch (c->mcause) {
      case 11: { // System Call From M-Mode
        #ifdef __riscv_e
        uintptr_t syscall_num = c->gpr[15]; // $a5
        #else
        uintptr_t syscall_num = c->gpr[17]; // $a7
        #endif
        c->mepc += 4; // mepc 指向 ecall，应该 +4 指向下一条指令
        if (syscall_num == (uintptr_t)(-1)) {
          ev.event = EVENT_YIELD;
          break;
        } else if (syscall_num >= 0 && syscall_num <= 19) { // navy-apps/libs/libos/src/syscall.h, SYS_exit=0, SYS_gettimeofday=19
          ev.event = EVENT_SYSCALL;
          break;
        }
      }
      case IRQ_TIMER: {
        // mepc == dnpc, 不需要改
        ev.event = EVENT_IRQ_TIMER;
        break;
      }
      default: ev.event = EVENT_ERROR; break;
    }

    c = user_handler(ev, c);
    assert(c != NULL);
  }

  __am_switch(c);
  return c;
}

extern void __am_asm_trap(void);

bool cte_init(Context*(*handler)(Event, Context*)) {
  // initialize exception entry
  asm volatile("csrw mtvec, %0" : : "r"(__am_asm_trap));

  // register event handler
  user_handler = handler;

  return true;
}

Context *kcontext(Area kstack, void (*entry)(void *), void *arg) {
  //return NULL;

  Context *context = (Context *)kstack.end - 1;
  /*
  初始化上下文
  先全部置零
  epc设置为entry-4，因为后续在__am_asm_trap要+4
  为了保证DiffTest的正确运行 和 mret之后开中断，mstatus设置为0x1880
  a0设置为arg
  */
  memset((void *)context, 0, sizeof(Context));
  context->mepc = (uintptr_t)entry;
  context->mstatus = 0x1880; // MPIE = 1u << 7;
  context->GPRx = (uintptr_t)arg;
  context->pdir = NULL;
  return context;
}

void yield() {
#ifdef __riscv_e
  asm volatile("li a5, -1; ecall");
#else
  asm volatile("li a7, -1; ecall");
#endif
}

bool ienabled() {
  return false;
}

void iset(bool enable) {
}
