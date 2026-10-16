#ifndef RISCV_H__
#define RISCV_H__

#include <stdint.h>

static inline uint8_t  inb(uintptr_t addr) { return *(volatile uint8_t  *)addr; }
static inline uint16_t inw(uintptr_t addr) { return *(volatile uint16_t *)addr; }
static inline uint32_t inl(uintptr_t addr) { return *(volatile uint32_t *)addr; }

static inline void outb(uintptr_t addr, uint8_t  data) { *(volatile uint8_t  *)addr = data; }
static inline void outw(uintptr_t addr, uint16_t data) { *(volatile uint16_t *)addr = data; }
static inline void outl(uintptr_t addr, uint32_t data) { *(volatile uint32_t *)addr = data; }

#define PTE_V 0x01 // 有效位
#define PTE_R 0x02 // 可读位
#define PTE_W 0x04 // 可写位
#define PTE_X 0x08 // 可执行位
#define PTE_U 0x10 // 用户模式可访问位
#define PTE_A 0x40 // 访问位,用于页面替换算法、统计页面使用情况或性能优化
#define PTE_D 0x80 // 脏位,表示页面是否被修改过,用于页面回写策略，优化页面换出性能

// 用户模式（应用程序）、特权模式（操作系统）、机器模式（固件）
enum { MODE_U, MODE_S, MODE_M = 3 };
#define MSTATUS_MXR  (1 << 19) // 允许读取可执行页
#define MSTATUS_SUM  (1 << 18) // 允许 S-mode 访问用户页

// ull - 类型后缀：unsigned long long,确保这是一个 64 位无符号整数
// SXL 字段的位置：mstatus 寄存器的第 35-34 位（2 位宽），10表示S-mode寄存器宽度为64位
// UXL 字段的位置：mstatus 寄存器的第 33-32 位（2 位宽），10表示U-mode寄存器宽度为64位
#if __riscv_xlen == 64
#define MSTATUS_SXL  (2ull << 34) // S-mode 寄存器宽度（64位）
#define MSTATUS_UXL  (2ull << 32) // U-mode 寄存器宽度（64位）
#define IRQ_TIMER 0x8000000000000007  // for riscv64
#else
#define MSTATUS_SXL  0
#define MSTATUS_UXL  0
#define IRQ_TIMER 0x80000007  // for riscv32
#endif

#endif
