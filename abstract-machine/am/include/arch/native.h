#ifndef ARCH_H__
#define ARCH_H__

#ifndef __USE_GNU
# define __USE_GNU
#endif

#include <ucontext.h>
// AM 的 native 平台上下文结构体，用于在真实硬件上保存和恢复程序执行状态
struct Context {
  uintptr_t ksp; // Kernel Stack Pointer(内核栈指针), 用于多任务切换时记录任务的栈位置; uintptr_t - 无符号整数类型，大小与指针相同
  void *vm_head; // Virtual Memory Head(虚拟内存头指针), 指向虚拟内存(页表)的起始地址,在支持虚拟内存的场景下，切换上下文时需要切换页表
  ucontext_t uc; // User Context(用户上下文),来自 <ucontext.h>, Linux/Unix 系统提供的标准上下文结构体,保存 CPU 的所有寄存器状态
#ifdef __x86_64__
  // skip the red zone of the stack frame, see the amd64 ABI manual for details
  uint8_t redzone[128];
#endif
};

#ifdef __x86_64__
#define GPR1 uc.uc_mcontext.gregs[REG_RDI]
#define GPR2 uc.uc_mcontext.gregs[REG_RSI]
#define GPR3 uc.uc_mcontext.gregs[REG_RDX]
#define GPR4 uc.uc_mcontext.gregs[REG_RCX]
#define GPRx uc.uc_mcontext.gregs[REG_RAX]
#elif defined(__aarch64__)
#define GPR1 uc.uc_mcontext.regs[0]
#define GPR2 uc.uc_mcontext.regs[1]
#define GPR3 uc.uc_mcontext.regs[2]
#define GPR4 uc.uc_mcontext.regs[3]
#define GPRx uc.uc_mcontext.regs[0]
#elif defined(__riscv)
// FIXME: may be wrong
// uc_mcontext: machine context,包含所有寄存器
// __gregs: 通用寄存器数组
#define GPR1 uc.uc_mcontext.__gregs[REG_A0] // 映射到 RISC-V 的 a0 寄存器（x10），函数调用约定中的第 1 个参数，也用作函数返回值
#define GPR2 uc.uc_mcontext.__gregs[REG_A0+1] // 映射到 a1 寄存器（x11）,函数调用约定中的第 2 个参数, REG_A0+1 假设寄存器在数组中是连续排列的
#define GPR3 uc.uc_mcontext.__gregs[REG_A0+2] // 映射到 a2 寄存器（x12）,函数调用约定中的第 3 个参数
#define GPR4 uc.uc_mcontext.__gregs[REG_A0+3] // 映射到 a3 寄存器（x13）,函数调用约定中的第 4 个参数
#define GPRx uc.uc_mcontext.__gregs[REG_A0+4] // 返回值寄存器 通常 RISC-V 的返回值应该用 a0（和 a1 用于 64 位返回值），这里映射到a4可能有问题
#else
#error Unsupported architecture
#endif

#undef __USE_GNU

#endif
