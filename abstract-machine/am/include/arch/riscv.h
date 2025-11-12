#ifndef ARCH_H__
#define ARCH_H__

#ifdef __riscv_e
#define NR_REGS 16
#else
#define NR_REGS 32
#endif
/*
mepc - Machine Exception Program Counter
机器模式异常程序计数器,保存发生异常/中断时的 PC 值,从异常返回时，会跳转到这个地址
mcause - Machine Cause Register
机器模式异常原因寄存器,记录异常/中断的原因代码,最高位：1=中断，0=异常,低位：具体原因
gpr[NR_REGS] - General Purpose Registers
通用寄存器数组,保存所有通用寄存器 (x0-x31 或 x0-x15)
mstatus - Machine Status Register
机器模式状态寄存器,包含全局中断使能位 (MIE)、之前的中断使能位 (MPIE) 等,控制特权级切换、中断使能等
void *pdir
Page Directory（页目录指针）,指向页表基地址,用于支持虚拟内存
*/
struct Context {
  // TODO: fix the order of these members to match trap.S
  uintptr_t mepc, mcause, gpr[NR_REGS], mstatus;
  void *pdir;
};

#ifdef __riscv_e
#define GPR1 gpr[15] // a5 因为 RV32E 只有 16 个寄存器,a5 是可用的最高参数寄存器
#else
#define GPR1 gpr[17] // a7 第 8 个参数寄存器,也用作系统调用号寄存器
#endif
/*
gpr[0] 是 x0 (zero) 寄存器，永远是 0
这样设计的原因可能是: 
这个 Context 主要用于系统调用和异常处理
在 AM 的系统调用约定中,a7 (GPR1)用于传递系统调用号，其他参数通过栈或其他方式传递，不需要使用 GPR2-GPR4
将它们指向 x0 是一种占位实现，读取时永远返回 0，写入时无效（x0 不可写），避免未定义行为
*/
#define GPR2 gpr[0]
#define GPR3 gpr[0]
#define GPR4 gpr[0]
#define GPRx gpr[0]

#endif
