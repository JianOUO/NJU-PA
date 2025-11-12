include $(AM_HOME)/scripts/isa/riscv.mk
include $(AM_HOME)/scripts/platform/nemu.mk
CFLAGS  += -DISA_H=\"riscv/riscv.h\"
# -march (Machine ARCHitecture) - 指定指令集架构
# rv32 i - 基础整数指令集, m - 乘除法扩展 (RV32M), _zicsr - CSR 指令扩展
# -mabi (Machine ABI) - 应用程序二进制接口
# 数据类型大小： i - int 是 32 位, l - long 是 32 位, p - pointer 是 32 位, 32 - 基本字长是 32 位
# 寄存器使用约定：整数参数：a0-a7（x10-x17），返回值：a0-a1，浮点参数：（ilp32 不使用浮点寄存器传参）
# 为什么是overwrite? 因为isa/riscv.mk中默认是64位，命令行中后出现的选项会覆盖前面的，所以这里的追加会覆盖默认值
COMMON_CFLAGS += -march=rv32im_zicsr -mabi=ilp32   # overwrite
# -m 选项 - 模拟（emulation）特定格式
# elf32 - ELF 32 位格式, l - 小端（little-endian）, riscv - RISC-V 架构
LDFLAGS       += -melf32lriscv                     # overwrite

AM_SRCS += riscv/nemu/start.S \
           riscv/nemu/cte.c \
           riscv/nemu/trap.S \
           riscv/nemu/vme.c
