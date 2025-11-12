AM_SRCS := platform/nemu/trm.c \
           platform/nemu/ioe/ioe.c \
           platform/nemu/ioe/timer.c \
           platform/nemu/ioe/input.c \
           platform/nemu/ioe/gpu.c \
           platform/nemu/ioe/audio.c \
           platform/nemu/ioe/disk.c \
           platform/nemu/mpe.c
# -ffunction-sections 将每个函数放入独立的段（如 .text.function_name）
# -fdata-sections 将每个全局变量/静态变量放入独立的段（如 .data.variable_name）
# 配合第14行的链接选项 --gc-sections 使用：将未使用的段删除
# 这样可以减小可执行文件大小，移除未使用的函数和数据，适合嵌入式/资源受限环境：如 NEMU 模拟器
CFLAGS    += -fdata-sections -ffunction-sections
CFLAGS    += -I$(AM_HOME)/am/src/platform/nemu/include
LDSCRIPTS += $(AM_HOME)/scripts/linker.ld
# --defsym=SYMBOL=VALUE 是链接器（ld）的选项，用于在链接时定义符号并设置其值。这些符号可以在链接脚本和 C 代码中使用。
# _pmem_start=0x80000000 物理内存起始地址,这是程序在 NEMU 模拟器中的加载地址
# _entry_offset=0x0 入口点偏移量,相对于 _pmem_start 的偏移,0x0 表示程序从物理内存起始位置开始
LDFLAGS   += --defsym=_pmem_start=0x80000000 --defsym=_entry_offset=0x0
# --gc-sections 删除未被引用的段, -e _start 指定程序入口点为 _start 符号
LDFLAGS   += --gc-sections -e _start
# NEMUFLAGS 变量用于传递给 NEMU 模拟器的命令行参数
# -l 指定日志输出文件，在monitor.c中解析
# 将 NEMU 的日志输出重定向到 nemu-log.txt 文件
# -b 启动批处理模式
NEMUFLAGS += -l $(shell dirname $(IMAGE).elf)/nemu-log.txt -b
# 主函数参数的最大长度（字节数）
MAINARGS_MAX_LEN = 64
# 主函数参数的占位符，在二进制文件中标记参数插入位置, 在insert-arg规则中替换为实际参数
MAINARGS_PLACEHOLDER = the_insert-arg_rule_in_Makefile_will_insert_mainargs_here
# 在trm.c中: static const char mainargs[MAINARGS_MAX_LEN] = TOSTRING(MAINARGS_PLACEHOLDER); // defined in CFLAGS
# 在编译阶段，预处理器展开后：static const char mainargs[64] = "the_insert-arg_rule_in_Makefile_will_insert_mainargs_here";
# 在链接阶段，生成.elf文件，包含占位符
# objcopy 将 .elf 转换为 .bin 二进制文件，bin 文件中仍包含占位符字符串
# 插入参数阶段，insert-arg规则中使用python脚本，将占位符替换为实际参数
# 运行时，mainargs 数组包含实际插入的命令行参数，传递给 main() 函数
CFLAGS += -DMAINARGS_MAX_LEN=$(MAINARGS_MAX_LEN) -DMAINARGS_PLACEHOLDER=$(MAINARGS_PLACEHOLDER)
# 传统程序（有操作系统）
# 用户执行
# ./myprogram arg1 arg2 arg3
# 操作系统的工作流程：
# 1. Shell 解析命令行参数
# 2. 调用 fork() 创建新进程
# 3. 调用 execve() 加载可执行文件
# 4. 操作系统设置进程的 argc 和 argv
# 5. 调用程序的 main(int argc, char *argv[])
# 嵌入式/裸机程序（NEMU 环境）
# 用户执行
# make run mainargs="arg1 arg2 arg3"
# NEMU 的工作流程：
# 1. 编译生成二进制文件 hello.bin
# 2. NEMU 模拟器直接将二进制文件加载到内存地址 0x80000000
# 3. 模拟器设置 PC（程序计数器）指向 _start
# 4. 程序从 _start 开始执行，没有操作系统！
# 5. 没有标准接口：无法通过 argc/argv 传递参数

insert-arg: image
	@python $(AM_HOME)/tools/insert-arg.py $(IMAGE).bin $(MAINARGS_MAX_LEN) $(MAINARGS_PLACEHOLDER) "$(mainargs)"

image: image-dep
# 使用 objdump 反汇编 ELF 文件
	@$(OBJDUMP) -d $(IMAGE).elf > $(IMAGE).txt
# 打印提示信息
	@echo + OBJCOPY "->" $(IMAGE_REL).bin
# 使用 objcopy 将 ELF 转换为二进制文件
# -S: 移除调试符号信息,减小文件大小
# --set-section-flags .bss=alloc,contents：设置 .bss 段的标志, alloc表示该段需要分配内存,contents 表示该段有内容(即使初始化为0)
# -O binary：输出格式为纯二进制（raw binary）
	@$(OBJCOPY) -S --set-section-flags .bss=alloc,contents -O binary $(IMAGE).elf $(IMAGE).bin

run: insert-arg
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) run ARGS="$(NEMUFLAGS)" IMG=$(IMAGE).bin

gdb: insert-arg
	$(MAKE) -C $(NEMU_HOME) ISA=$(ISA) gdb ARGS="$(NEMUFLAGS)" IMG=$(IMAGE).bin

.PHONY: insert-arg
