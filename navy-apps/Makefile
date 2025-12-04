# Makefile for Navy Applications and Libraries

### *Get a more readable version of this Makefile* by `make html` (requires python-markdown)
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
.PHONY: html

## 1. Basic Setup and Checks

### Default to build an application
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = app
  .DEFAULT_GOAL = app
endif

### Override checks when `make clean/clean-all/html/init`
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html|init),)

### Print build info message
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ISA)])

### Check: environment variable `$NAVY_HOME` looks sane
ifeq ($(wildcard $(NAVY_HOME)/libs/libos/src/syscall.h),)
  $(error $$NAVY_HOME must be a Navy-apps repo)
endif

### Check: environment variable `$ISA` must be in the supported list
ISAS = $(basename $(notdir $(shell ls $(NAVY_HOME)/scripts/*.mk)))
ifeq ($(filter $(ISAS), $(ISA)), )
  $(error Expected $$ISA in {$(ISAS)}, Got "$(ISA)")
endif

### Checks end here
endif

## 2. General Compilation Targets

### Create the destination directory (`build/$ISA`)
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ISA)
$(shell mkdir -p $(DST_DIR))

### Compilation targets (application or archive)
APP     ?= $(WORK_DIR)/build/$(NAME)-$(ISA)
ARCHIVE  = $(WORK_DIR)/build/$(NAME)-$(ISA).a


### Add default libraries for ISA != native
ifneq ($(ISA), native)
LIBS += libc libos
CFLAGS += -U_FORTIFY_SOURCE  # fix compile error in Newlib on ubuntu
else
# WL = -Wl,：设置链接器选项前缀
# -Wl,：告诉 gcc 将后续参数传递给链接器
WL = -Wl,
endif

### 32-bit ISA may need compiler-rt to support 64-bit mul/div
ifneq ($(findstring $(ISA), x86|mips32|riscv32|riscv32e|loongarch32r),)
LIBS += compiler-rt
endif

### All libraries to add to the include path
LIBS_INC := $(sort $(LIBS) $(LIB_DEP))

# 构建静态库时，只构建依赖库，不构建当前库本身，避免循环依赖
ifeq ($(MAKECMDGOALS),archive)
LIBS := $(LIB_DEP) # overwrite here to only build and archive the dependent libraries
endif

### Files to be linked: object files (`.o`) and libraries (`.a`)
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
LIBS     := $(sort $(LIBS)) # lazy evaluation ("=") causes infinite recursions
LINKAGE  += $(OBJS)  # library archives are added by LIB_TEMPLATE below

## 3. General Compilation Flags

### (Cross) compilers, e.g., mips-linux-gnu-g++
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar

### Compilation flags
INC_PATH += $(WORK_DIR)/include $(foreach l,$(LIBS_INC),$(NAVY_HOME)/libs/$(l)/include/)
INCFLAGS += $(addprefix -I, $(INC_PATH))

# -MMD：生成.d依赖文件
# static：静态链接所有库, 可执行文件包含所有依赖，不依赖动态库
# -fno-asynchronous-unwind-tables：禁用异步栈展开表，减小代码大小
# -fno-builtin：禁用内建函数优化, 不将标准库函数替换为内建版本, 避免与自定义实现(Newlib?)冲突
# -fno-stack-protector：禁用栈保护
CFLAGS   += -O2 -MMD $(INCFLAGS) -static -D__NAVY__ \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector
# -fno-rtti：禁用运行时类型信息, 不能使用 dynamic_cast、typeid
# -fno-exceptions：禁用 C++ 异常, 不能使用 try/catch/throw
CXXFLAGS +=  $(CFLAGS) -fno-rtti -fno-exceptions
ASFLAGS  +=  $(CFLAGS)
# 为什么需要这些选项？
# Navy-apps 运行在受限环境：
# 无动态链接器：需要静态链接
# 无异常处理：禁用异常和 RTTI
# 代码大小敏感：禁用不必要的功能
# 自定义运行时：使用自定义 libc，禁用内建函数


## 4. ISA-Specific Configurations

### Paste in ISA-specific configurations (e.g., from `scripts/x86.mk`)
-include $(NAVY_HOME)/scripts/$(ISA).mk

## 5. Compilation Rules

### Rule (compile): a single `.c` -> `.o` (gcc)
$(DST_DIR)/%.o: %.c
	@mkdir -p $(dir $@) && echo + CC $<
	@$(CC) -std=gnu11 $(CFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cc` -> `.o` (g++)
$(DST_DIR)/%.o: %.cc
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.cpp` -> `.o` (g++)
$(DST_DIR)/%.o: %.cpp
	@mkdir -p $(dir $@) && echo + CXX $<
	@$(CXX) -std=c++17 $(CXXFLAGS) -c -o $@ $(realpath $<)

### Rule (compile): a single `.S` -> `.o` (gcc, which calls as)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

# $(1): library name
define LIB_TEMPLATE =
$$(NAVY_HOME)/libs/$(1)/build/$(1)-$$(ISA).a: force
	@$$(MAKE) -s -C $$(NAVY_HOME)/libs/$(1) archive
LINKAGE += $$(NAVY_HOME)/libs/$(1)/build/$(1)-$$(ISA).a
endef

### Rule (recursive make): build dependent libraries (libc, libos, ...)
$(foreach lib, $(LIBS), $(eval $(call LIB_TEMPLATE,$(lib))))

### Rule (link): objects (`*.o`) and libraries (`*.a`) -> `$(APP)`, the final ELF binary to be packed into application (ld)
# native 模式（WL = -Wl,）
# g++ -o app -Wl,--start-group obj1.o obj2.o -Wl,--end-group
# 传递给链接器 --start-group 和 --end-group
# 非 native 模式（WL 未定义，为空）
# riscv32-linux-gnu-ld -o app --start-group obj1.o obj2.o libc.a libos.a --end-group
# 直接使用 --start-group 和 --end-group
# 直接调用 ld，不需要 -Wl, 前缀
$(APP): $(LINKAGE)
	@echo + LD "->" $(shell realpath $@ --relative-to .)
	@$(LD) $(LDFLAGS) -o $@ $(WL)--start-group $^ $(WL)--end-group

### Rule (archive): objects (`*.o`) -> `ARCHIVE.a` (ar)
$(ARCHIVE): $(LINKAGE)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@$(AR) rcs --thin $@ $^

### Rule (`#include` dependencies): paste in `.d` files generated by gcc on `-MMD`
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## 6. Miscellaneous

### Pull newlib from github if it does not exist
ifeq ($(wildcard $(NAVY_HOME)/libs/libc/Makefile),)
  $(shell cd $(NAVY_HOME)/libs && git clone git@github.com:NJU-ProjectN/newlib-navy.git libc)
endif

### Build order control
app: $(APP)
archive: $(ARCHIVE)
.PHONY: app archive

### Force to rebuild a rule
force:
.PHONY: force

### Install an application to fsimg
install: app
	@echo + INSTALL "->" $(NAME)
	@mkdir -p $(NAVY_HOME)/fsimg/bin
	@cp $(APP) $(NAVY_HOME)/fsimg/bin/$(NAME)

.PHONY: install

### Clean a single project (remove `build/`)
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/ build/
.PHONY: clean

### Clean all sub-projects within depth 2 (and ignore errors)
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -maxdepth 3 -name Makefile))
# 先执行 $(CLEAN_ALL) 中的每个目标，清理所有子项目
# 再执行 clean，清理当前项目
clean-all: $(CLEAN_ALL) clean
# rm -f：强制删除，忽略不存在的文件
	-@rm -f $(NAVY_HOME)/fsimg/bin/*
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
.PHONY: clean-all $(CLEAN_ALL)

### Build fsimg and ramdisk for Nanos-lite
APPS =
TESTS = dummy hello file-test timer-test event-test

fsimg: $(addprefix apps/, $(APPS)) $(addprefix tests/, $(TESTS))
	-for t in $^; do $(MAKE) -s -C $(NAVY_HOME)/$$t install; done

RAMDISK = build/ramdisk.img
RAMDISK_H = build/ramdisk.h
$(RAMDISK): fsimg
# 查找fsimg目录下所有的普通文件（不包括目录）
# FSIMG_FILES：存储所有文件路径的变量
	$(eval FSIMG_FILES := $(shell find -L ./fsimg -type f))
# $(@D)：目标文件的目录部分 (build/)
	@mkdir -p $(@D)
# 将所有文件按顺序拼接成一个镜像文件
	@cat $(FSIMG_FILES) > $@
# truncate：调整文件大小
# -s \%512：将文件大小向上对齐到 512 字节的倍数
# \%512：模 512（Makefile 中 % 需要转义为 \%）
# 将镜像大小对齐到扇区大小（512 字节），便于块设备访问。
	@truncate -s \%512 $@
# 为文件信息头文件添加注释：文件路径, 文件大小, 磁盘偏移量
	@echo "// file path, file size, offset in disk" > $(RAMDISK_H)
# wc -c ./fsimg/bin/hello ./fsimg/bin/dummy 统计每个文件的字节数
# 1024 ./fsimg/bin/hello
# 512  ./fsimg/bin/dummy
# 1536 total
# grep -v 'total$$' 删除 "total" 行
# grep -v：反向匹配(排除)
# 'total$$'：匹配以 "total" 结尾的行($$ 在 Makefile 中表示单个 $)
# sed -e 's+ ./fsimg+ +' 简化路径，去掉 ./fsimg 前缀
# 将文件路径中的 "./fsimg/" 替换为 " "
# awk -v sum=0 '{print "\x7b\x22" $$2 "\x22\x2c " $$1 "\x2c " sum "\x7d\x2c";sum += $$1}'
# -v sum=0 初始化变量 sum = 0（累计偏移量）
# $$2：第二个字段（文件名）$$1：第一个字段（文件大小）
# \x7b={  \x22="  \x7d=}  \x2c=,
# 最终输出重定向到 build/ramdisk.h
# // file path, file size, offset in disk
# {"bin/hello", 1024, 0},
# {"bin/dummy", 512, 1024},
# {"share/files/num", 10, 1536},
	@wc -c $(FSIMG_FILES) | grep -v 'total$$' | sed -e 's+ ./fsimg+ +' | awk -v sum=0 '{print "\x7b\x22" $$2 "\x22\x2c " $$1 "\x2c " sum "\x7d\x2c";sum += $$1}' >> $(RAMDISK_H)

ramdisk: $(RAMDISK)

.PHONY: fsimg ramdisk
