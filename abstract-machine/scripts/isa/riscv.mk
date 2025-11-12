# 指定 RISC-V 交叉编译工具链的前缀
# 即使编译 32 位程序，也使用 riscv64 工具链，通过 -march 和 -mabi 参数控制目标位数
CROSS_COMPILE := riscv64-linux-gnu-
# -fno-pic: 禁用位置无关代码（Position Independent Code）
# -march=rv64g RISC-V 64 位通用配置(imafd_zicsr_zifencei)
# -mcmodel=medany 代码模型：中等代码模型, 允许代码和数据放在 64 位地址空间的任意位置, 使用 PC 相对寻址，适合需要灵活内存布局的场景
# -mstrict-align：强制对齐访问, 要求所有内存访问必须对齐, 避免未对齐访问导致的性能问题或错误
COMMON_CFLAGS := -fno-pic -march=rv64g -mcmodel=medany -mstrict-align
# -static：静态链接，所有库都链接到可执行文件中，不依赖动态库，适合裸机环境（没有动态链接器）
CFLAGS        += $(COMMON_CFLAGS) -static
# -O0：不进行优化，保持汇编代码的原始结构
ASFLAGS       += $(COMMON_CFLAGS) -O0
# -m 选项 - 模拟（emulation）特定格式
# elf64 - ELF 64 位格式, l - 小端（little-endian）, riscv - RISC-V 架构
LDFLAGS       += -melf64lriscv

# overwrite ARCH_H defined in $(AM_HOME)/Makefile
ARCH_H := arch/riscv.h
