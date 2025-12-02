#HAS_NAVY = 1
RAMDISK_FILE = build/ramdisk.img

NAME = nanos-lite
# -L：跟随符号链接; -name "*.c"：匹配 .c 文件; -o：逻辑或（OR）
SRCS = $(shell find -L ./src/ -name "*.c" -o -name "*.cpp" -o -name "*.S")
include $(AM_HOME)/Makefile

ifeq ($(ARCH),native)
# 这里会将 AM_HOME 中的 Makefile 中定义的 ISA=native 覆盖
ISA = am_native
else
INC_PATH += include $(NAVY_HOME)/libs/libc/include
endif

# 步骤 1: ramdisk.img 被更新
# build/ramdisk.img  (时间戳: 2024-01-01 10:00:00)
# 步骤 2: Make 检查显式依赖规则
# ./src/resources.S: build/ramdisk.img
# 比较时间戳:
#   resources.S: 2024-01-01 09:00:00
#   ramdisk.img: 2024-01-01 10:00:00
# resources.S < ramdisk.img → 需要执行规则
# 步骤 3: 执行 touch 命令
# touch ./src/resources.S
# resources.S 时间戳更新为: 2024-01-01 10:00:01
# 步骤 4: Make 检查模式规则（关键步骤！）
# build/riscv32-nemu/resources.o: ./src/resources.S
# 比较时间戳:
#   resources.o: 2024-01-01 09:00:00 (旧的目标文件)
#   resources.S: 2024-01-01 10:00:01 (刚更新的源文件)
# resources.o < resources.S → 需要重新编译！
# 步骤 5: 执行编译命令
# gcc -c -o build/riscv32-nemu/resources.o ./src/resources.S
# 汇编器处理 .incbin 指令，包含新的 ramdisk.img 内容
./src/resources.S: $(RAMDISK_FILE)
	@touch $@

ifeq ($(HAS_NAVY),)
files = $(RAMDISK_FILE) src/files.h src/syscall.h
# create an empty file if it does not exist
# 外层: $(foreach f,$(files),...)
# foreach：循环函数; f：循环变量; $(files)：要遍历的列表
# 中层：$(if ...)
# $(wildcard $f)：检查文件是否存在
# 第一个参数为空：如果文件存在，什么都不做
# $(shell touch $f)：如果文件不存在，执行 touch 创建
$(foreach f,$(files),$(if $(wildcard $f),, $(shell touch $f)))
else

ifeq ($(wildcard $(NAVY_HOME)/libs/libos/src/syscall.h),)
  $(error $$NAVY_HOME must be a Navy-apps repo)
endif

# 步骤 1: 在 Navy-apps 目录中构建 ramdisk
# cd /path/to/navy-apps
# make -s ISA=riscv32 ramdisk
# 生成文件：
#   /path/to/navy-apps/build/ramdisk.img
#   /path/to/navy-apps/build/ramdisk.h
# 步骤 2: 创建符号链接
# ln -sf：创建符号链接; -s：符号链接; -f：强制（覆盖已存在的链接）
# ramdisk.img → /path/to/navy-apps/build/ramdisk.img  (符号链接)
# files.h → /path/to/navy-apps/build/ramdisk.h (符号链接)
# syscall.h → /path/to/navy-apps/libs/libos/src/syscall.h (符号链接)
update:
	$(MAKE) -s -C $(NAVY_HOME) ISA=$(ISA) ramdisk
	@ln -sf $(NAVY_HOME)/build/ramdisk.img $(RAMDISK_FILE)
	@ln -sf $(NAVY_HOME)/build/ramdisk.h src/files.h
	@ln -sf $(NAVY_HOME)/libs/libos/src/syscall.h src/syscall.h

.PHONY: update
endif
