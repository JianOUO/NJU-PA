CROSS_COMPILE = riscv64-linux-gnu-
# 链接地址选择
LNK_ADDR = $(if $(VME), 0x40000000, 0x83000000)
# -fno-pic: 禁用位置无关代码（Position Independent Code）
# Navy-apps 程序加载到固定地址，不需要 PIC
# -march=rv64g: 指定 RISC-V 64 位通用配置(imafd_zicsr_zifencei)
# -mcmodel=medany: 中等任意代码模型(Medium Any), 代码和数据可放在 64 位地址空间的任意位置, 使用 PC 相对寻址（±2GB 范围）
CFLAGS  += -fno-pic -march=rv64g -mcmodel=medany
# --no-relax: 禁用链接器松弛优化（Linker Relaxation）
# RISC-V 链接器默认会进行指令优化(如将 auipc + addi 优化为单个指令), 某些情况下松弛优化可能导致问题，禁用以确保兼容性
# -Ttext-segment $(LNK_ADDR): 指定文本段（代码段）的加载地址
LDFLAGS += --no-relax -Ttext-segment $(LNK_ADDR)
