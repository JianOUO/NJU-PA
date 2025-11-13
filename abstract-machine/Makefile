# Makefile for AbstractMachine Kernels and Libraries

### *Get a more readable version of this Makefile* by `make html` (requires python-markdown)
#  sed - Stream Editor(流编辑器)用于对文本进行查找和替换操作
# s/pattern/replacement/g - 替换命令格式：pattern 要匹配的模式，replacement 替换的内容，g 表示全局替换
# ^ - 行首锚点,匹配每行的开始位置,确保只匹配行首的字符
# \( 和 \) - 在 sed 的基本正则表达式中表示捕获组, 括号内匹配的内容会被"捕获"，可以在替换部分引用,捕获的内容保存为 \1（第一个捕获组）
# [ ] - 字符类，匹配方括号内的任意一个字符, 如 [0-9] 匹配任意一个数字, ^ - 在字符类内部表示"否定"（注意：与行首的 ^ 含义不同）, [^#] - 匹配任何一个不是 # 的字符
# 替换部分： \1, 在匹配的内容前添加 4 个空格,Markdown 语法中，缩进 4 个空格表示代码块 
# \1 - 反向引用 引用第一个捕获组 \( ... \) 中匹配的内容,保留原始匹配的字符
# 用"4个空格 + 原字符"替换原字符
html:
	cat Makefile | sed 's/^\([^#]\)/    \1/g' | markdown_py > Makefile.html
.PHONY: html

## 1. Basic Setup and Checks

### Default to create a bare-metal kernel image
# MAKECMDGOALS - Make 的内置变量,包含命令行指定的目标,如果没有指定目标，值为空
# make  MAKECMDGOALS = ""（空）
# make clean  MAKECMDGOALS = "clean"
# make image run MAKECMDGOALS = "image run"
# 当用户只输入 make 时：MAKECMDGOALS = image,手动设置变量值,供后续使用
# .DEFAULT_GOAL = image,Make 的特殊变量,指定默认构建目标,等价于 make image
# image: 构建裸机内核镜像,生成可执行的二进制文件,可以在 NEMU、真实硬件等平台运行
ifeq ($(MAKECMDGOALS),)
  MAKECMDGOALS  = image
  .DEFAULT_GOAL = image
endif

### Override checks when `make clean/clean-all/html`
# 在字符串 "clean|clean-all|html" 中查找 MAKECMDGOALS,如果未找到，返回""
# 如果目标是清理或生成文档，跳过环境检查
ifeq ($(findstring $(MAKECMDGOALS),clean|clean-all|html),)

### Print build info message
# NAME是项目名，在每个子项目的 Makefile 中定义的
# 示例 1：hello 项目
# am-kernels/kernels/hello/Makefile
# NAME = hello
# SRCS = hello.c
# include $(AM_HOME)/Makefile
# 子项目先定义 NAME 和 SRCS, 然后 include 主 Makefile

# 例子 1：构建 hello 程序
# $ cd am-kernels/kernels/hello
# $ make ARCH=riscv32-nemu
# Building hello-image [riscv32-nemu]
# 例子 2：运行程序
# make run ARCH=native
# Building hello-run [native]
$(info # Building $(NAME)-$(MAKECMDGOALS) [$(ARCH)])

### Check: environment variable `$AM_HOME` looks sane
ifeq ($(wildcard $(AM_HOME)/am/include/am.h),)
  $(error $$AM_HOME must be an AbstractMachine repo)
endif

### Check: environment variable `$ARCH` must be in the supported list
ARCHS = $(basename $(notdir $(shell ls $(AM_HOME)/scripts/*.mk)))
# $(filter pattern..., text) 从 text 中提取匹配 pattern 的单词
# $(filter riscv32-nemu riscv64-nemu native x86_64-qemu x86-nemu, riscv32-nemu)
#          ^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^ ^^^^^^^^^^^^
#                        pattern(s)                                    text
# PATTERN 可以是多个模式,逐个检查 TEXT 中的单词是否匹配任一PATTERN
ifeq ($(filter $(ARCHS), $(ARCH)), )
  $(error Expected $$ARCH in {$(ARCHS)}, Got "$(ARCH)")
endif

### Extract instruction set architecture (`ISA`) and platform from `$ARCH`. Example: `ARCH=x86_64-qemu -> ISA=x86_64; PLATFORM=qemu`
# 将 ARCH 中的 "-" 替换为 " ", 将 "riscv32-nemu" 转换为 "riscv32 nemu"
ARCH_SPLIT = $(subst -, ,$(ARCH))
# 提取第一个单词,即 ISA, 如 "riscv32"
ISA        = $(word 1,$(ARCH_SPLIT))
# 提取第二个单词,即 PLATFORM, 如 "nemu"
PLATFORM   = $(word 2,$(ARCH_SPLIT))

### Check if there is something to build
# $(flavor variable) - 查询变量的定义类型
# 功能：返回变量的"风味"（定义方式）
# undefined: 变量未定义
# simple: 简单展开（:=）
# recursive: 递归展开（=）
# append: 追加展开（+=）
ifeq ($(flavor SRCS), undefined)
  $(error Nothing to build)
endif

### Checks end here
endif

## 2. General Compilation Targets

### Create the destination directory (`build/$ARCH`)
# 获取当前工作目录的绝对路径，比如 am 或者 klib
WORK_DIR  = $(shell pwd)
DST_DIR   = $(WORK_DIR)/build/$(ARCH)
$(shell mkdir -p $(DST_DIR))

### Compilation targets (a binary image or archive)
# 定义可执行文件的相对路径
IMAGE_REL = build/$(NAME)-$(ARCH)
# 转化为绝对路径
IMAGE     = $(abspath $(IMAGE_REL))
# 定义静态库的输出路径
# .a 文件 - 静态库（Archive）：多个 .o 目标文件的集合,可以被其他程序链接,使用 ar 命令创建
ARCHIVE   = $(WORK_DIR)/build/$(NAME)-$(ARCH).a

### Collect the files to be linked: object files (`.o`) and libraries (`.a`)
# 将源文件（.c, .s）转换为目标文件路径（.o）
OBJS      = $(addprefix $(DST_DIR)/, $(addsuffix .o, $(basename $(SRCS))))
# 添加必需的库：am 和 klib,使用 sort 排序并去重,使用 := 避免递归展开问题
LIBS     := $(sort $(LIBS) am klib) # lazy evaluation ("=") causes infinite recursions
# 初始化为目标文件列表,后续会添加库文件,最终用于链接命令
# library archives are added by LIB_TEMPLATE below
LINKAGE   = $(OBJS)

## 3. General Compilation Flags

### (Cross) compilers, e.g., mips-linux-gnu-g++
AS        = $(CROSS_COMPILE)gcc
CC        = $(CROSS_COMPILE)gcc
CXX       = $(CROSS_COMPILE)g++
LD        = $(CROSS_COMPILE)ld
AR        = $(CROSS_COMPILE)ar
OBJDUMP   = $(CROSS_COMPILE)objdump
OBJCOPY   = $(CROSS_COMPILE)objcopy
READELF   = $(CROSS_COMPILE)readelf

### Compilation flags
# 这行代码收集所有的头文件搜索路径，分两部分：
# 第一部分：$(WORK_DIR)/include，当前项目的 include 目录
# 第二部分：$(addsuffix /include/, $(addprefix $(AM_HOME)/, $(LIBS)))，库的 include 目录
INC_PATH += $(WORK_DIR)/include $(addsuffix /include/, $(addprefix $(AM_HOME)/, $(LIBS)))
# -I - GCC 的头文件搜索路径选项
INCFLAGS += $(addprefix -I, $(INC_PATH))

ARCH_H := arch/$(ARCH).h
# C 编译标志
# -D__ISA__=\"riscv32\", 在C代码中等价于：#define __ISA__ "riscv32"
# -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__
# echo riscv32 | tr a-z A-Z 输出：RISCV32
# 展开 -D__ISA_RISCV32__，在 C 代码中等价于：#define __ISA_RISCV32__ 1
# -D__ARCH__=riscv32-nemu, 在 C 代码中等价于：#define __ARCH__ riscv32-nemu  // 注意：不是字符串
# $(shell echo riscv32-nemu | tr a-z A-Z | tr - _) 输出：RISCV32_NEMU
# 展开-D__ARCH_RISCV32_NEMU，在C代码中等价于：#define __ARCH_RISCV32_NEMU 1
# ARCH_H := arch/$(ARCH).h, 展开为 ARCH_H := arch/riscv32-nemu.h
# -DARCH_H=\"arch/riscv32-nemu.h\", 在C代码中相当于：#define ARCH_H "arch/riscv32-nemu.h"
# 裸机编译选项
# -fno-asynchronous-unwind-tables：不生成异步栈展开表，减小二进制文件大小，裸机环境不需要异常处理
# -fno-builtin：禁用内建函数优化，gcc 不会将 strlen() 等替换为内建版本，使用我们自己实现的 klib 函数
# -fno-stack-protector：禁用栈保护(Stack Canary),裸机环境没有栈保护支持,减小代码大小
# -Wno-main:禁用 main 函数相关警告,AM 程序的 main 可能有特殊签名
# -U_FORTIFY_SOURCE: 取消定义 _FORTIFY_SOURCE 宏,禁用 glibc 的安全检查,裸机环境不适用
# -fvisibility=hidden: 默认符号可见性为隐藏,减小符号表大小,提高性能
CFLAGS   += -O2 -MMD -Wall -Werror $(INCFLAGS) \
            -D__ISA__=\"$(ISA)\" -D__ISA_$(shell echo $(ISA) | tr a-z A-Z)__ \
            -D__ARCH__=$(ARCH) -D__ARCH_$(shell echo $(ARCH) | tr a-z A-Z | tr - _) \
            -D__PLATFORM__=$(PLATFORM) -D__PLATFORM_$(shell echo $(PLATFORM) | tr a-z A-Z | tr - _) \
            -DARCH_H=\"$(ARCH_H)\" \
            -fno-asynchronous-unwind-tables -fno-builtin -fno-stack-protector \
            -Wno-main -U_FORTIFY_SOURCE -fvisibility=hidden
# C++ 编译标志
# -ffreestanding: 独立环境编译模式,告诉编译器这是裸机程序,不依赖标准库,main() 不一定是入口点
# -fno-rtti: 禁用运行时类型信息（Run-Time Type Information）,减小代码大小, 不能使用 dynamic_cast, typeid
# -fno-exceptions: 禁用 C++ 异常,减小代码大小,不能使用 try/catch/throw
CXXFLAGS +=  $(CFLAGS) -ffreestanding -fno-rtti -fno-exceptions
# 汇编标志
# -MMD - 生成依赖文件，$(INCFLAGS) - 包含路径
ASFLAGS  += -MMD $(INCFLAGS)
# 链接器标志
# -z noexecstack: 标记栈为不可执行,防止栈溢出攻击
# $(addprefix -T, $(LDSCRIPTS)): -T - 指定链接器脚本
# 链接器脚本的作用：控制内存布局, 指定代码段、数据段位置, 定义入口点
LDFLAGS  += -z noexecstack $(addprefix -T, $(LDSCRIPTS))

## 4. Arch-Specific Configurations

### Paste in arch-specific configurations (e.g., from `scripts/x86_64-qemu.mk`)
-include $(AM_HOME)/scripts/$(ARCH).mk

## 5. Compilation Rules

### Rule (compile): a single `.c` -> `.o` (gcc)
# mkdir -p - 递归创建目录，如果目录已存在，不报错；$@ - 自动变量：目标文件名；$(dir names) - 提取目录部分
# + - 表示正在执行的操作； CC - 编译器标识; $< - 自动变量：第一个依赖文件
# $(CC) - C 编译器; -std=gnu11 - C语言标准：C11 - C 语言 2011 标准，gnu - GNU 扩展，允许使用 GCC 特有的扩展；
# -c - 只编译不链接；-o $@ - 指定输出文件；$(realpath $<) - 源文件的绝对路径
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

### Rule (compile): a single `.S` -> `.o` (gcc, which preprocesses and calls as)
$(DST_DIR)/%.o: %.S
	@mkdir -p $(dir $@) && echo + AS $<
	@$(AS) $(ASFLAGS) -c -o $@ $(realpath $<)

ifeq ($(MAKECMDGOALS),archive)
### Rule (archive): objects (`*.o`) -> `ARCHIVE.a` (ar)
# realpath $@ --relative-to . 返回相对于当前目录的路径
# $(AR) - 归档器; r - replace - 添加(不存在)/替换文件(已存在)到归档，c - create - 创建归档（如果不存在），
# s - symbol index - 创建符号索引表（加速链接）
# $@ - 目标文件(归档文件),$^ - 所有依赖(所有 .o 文件)
# 所有 .o 文件打包成一个 .a 文件
$(ARCHIVE): $(OBJS)
	@echo + AR "->" $(shell realpath $@ --relative-to .)
	@$(AR) rcs $@ $^
else
# $(1): 模板参数，这里为library name
# $$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a: force
# 在 define 中，$$ → $(转义),$(1) → 模板参数(库名)
# force 是一个总是被认为是"过期"的伪目标,确保库总是被检查(可能需要重新构建),在305行
# $(MAKE) - Make 程序本身, -s - silent（静默模式，不显示进入/离开目录的消息）, -C dir - 切换到目录 dir 再执行
# 递归调用 make archive , 走 if 分支
# LINKAGE += $$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a: 添加到链接列表
define LIB_TEMPLATE =
$$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a: force
	@bear -- $$(MAKE) -B -s -C $$(AM_HOME)/$(1) archive
LINKAGE += $$(AM_HOME)/$(1)/build/$(1)-$$(ARCH).a
endef

### Rule (recursive make): build a dependent library (am, klib, ...)
# 为每个依赖的库（am, klib 等）自动生成构建规则
# 第一层：$(call LIB_TEMPLATE,$(lib))：调用模版，例如 $(call LIB_TEMPLATE, am)
# 将 $(1) 替换为 am，展开为：
#$(AM_HOME)/am/build/am-$(ARCH).a: force
#	@$(MAKE) -s -C $(AM_HOME)/am archive
#LINKAGE += $(AM_HOME)/am/build/am-$(ARCH).a
# 第二层：$(eval text) - 将 text 作为 Makefile 代码执行
# 如果没有eval,只是文本展开，不会创建规则,需要 eval 才能让 Make 解析和执行
# 第三层：$(foreach var, list, text) - 循环遍历
# variable - 循环变量，list - 要遍历的列表，text - 对每个元素执行的操作
$(foreach lib, $(LIBS), $(eval $(call LIB_TEMPLATE,$(lib))))
endif

### Rule (link): objects (`*.o`) and libraries (`*.a`) -> `IMAGE.elf`, the final ELF binary to be packed into image (ld)
# 实例：IMAGE = /home/.../am-kernels/kernels/hello/build/hello-riscv32-nemu
# 依赖 1：$(LINKAGE)
# LINKAGE = build/riscv32-nemu/hello.o \                    # 项目的目标文件
#           /home/.../am/build/am-riscv32-nemu.a \          # am 库
#           /home/.../klib/build/klib-riscv32-nemu.a        # klib 库
# $(LDSCRIPTS) - 链接器脚本
$(IMAGE).elf: $(LINKAGE) $(LDSCRIPTS)
# 创建镜像提示
	@echo \# Creating image [$(ARCH)]
# 链接操作提示
	@echo + LD "->" $(IMAGE_REL).elf
# 判断是否是 native 架构
ifneq ($(filter $(ARCH),native),)
# 如果是 native 架构，使用 g++ 链接器,自动链接 C++ 运行时库,处理 C++ 的启动代码
# -Wl,option - 传递选项给链接器,后面的选项传给ld,逗号分隔多个选项
# --whole-archive - 强制包含归档中的所有目标文件
# -no-whole-archive - 结束全归档模式
# g++ ... -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX)
#                             ^^^^^^^^                         ^^^^^^^^^^^^^
#                             我们的库                        	系统库
# 只对我们的库使用 whole-archive
# 系统库（如 libc）使用正常模式
	@$(CXX) -o $@ -Wl,--whole-archive $(LINKAGE) -Wl,-no-whole-archive $(LDFLAGS_CXX)
else
# 直接使用 $(LD) 链接器, 例如riscv32-linux-gnu-ld, 完全控制链接过程，使用自定义链接脚本
# 不使用gcc/g++, 因为是裸机环境，不需要C/C++运行时库
# LDFLAGS  += -z noexecstack $(addprefix -T, $(LDSCRIPTS))
# --start-group ... --end-group - 循环依赖解决
	@$(LD) $(LDFLAGS) -o $@ --start-group $(LINKAGE) --end-group
endif

### Rule (`#include` dependencies): paste in `.d` files generated by gcc on `-MMD`
# -include files 包含其他 Makefile 文件, 如果文件不存在，静默忽略（不报错）
# 第一次编译时，.d 文件还不存在,使用 include 会报错,使用 -include 会忽略（第一次编译后会生成）
# .d文件是 gcc 的 -MMD 选项自动生成，记录了目标文件-源文件-头文件 的依赖关系
# 包含 .d 文件后，Make 知道：
# 原始规则（模式规则）
# build/riscv32-nemu/hello.o: hello.c
# 加上 .d 文件的内容
# build/riscv32-nemu/hello.o: hello.c \
#  am.h \
#  klib.h \
#  config.h
# 现在如果任何头文件改变，都会重新编译
-include $(addprefix $(DST_DIR)/, $(addsuffix .d, $(basename $(SRCS))))

## 6. Miscellaneous

### Build order control
# image不直接依赖 .elf 文件,而是依赖中间目标 image-dep
# 原因1：分离关注点
# image-dep: $(IMAGE).elf
#   可以在这里添加 ELF 相关的操作，如：strip（去除符号表）
# image: image-dep
#   在这里添加镜像相关的操作，如：objcopy（生成二进制）
# 原因2：后续脚本扩展
# 架构特定的脚本可能会添加额外的处理，通过 image-dep 作为锚点，不影响 image 目标
image: image-dep
archive: $(ARCHIVE)
image-dep: $(IMAGE).elf
.PHONY: image image-dep archive run

### Force to rebuild a rule
# 空规则，无依赖，无命令
force:
# 总是过期 - .PHONY 目标总是被认为需要重建
# 传递过期 - 依赖 force 的目标也变成过期
# 强制检查 - 总是执行规则（但不一定重新构建）
.PHONY: force

### Clean a single project (remove `build/`)
clean:
	rm -rf Makefile.html $(WORK_DIR)/build/
.PHONY: clean

### Clean all sub-projects within depth 2 (and ignore errors)
# 递归清理所有子项目，忽略错误
# 查找所有子项目
CLEAN_ALL = $(dir $(shell find . -mindepth 2 -name Makefile))
# clean-all 目标, 依赖$(CLEAN_ALL) - 所有子项目目录（作为目标），clean - 当前项目的清理目标
# 先执行 $(CLEAN_ALL) 中的每个目标，清理所有子项目
# 再执行 clean，清理当前项目
clean-all: $(CLEAN_ALL) clean
# 子项目清理规则，-@ 忽略错误，-s 静默模式，-C $@ 切换到子项目目录，clean 执行子项目清理目标
$(CLEAN_ALL):
	-@$(MAKE) -s -C $@ clean
# 声明为伪目标：clean-all - 主清理目标 
# $(CLEAN_ALL) - 所有子项目目录(虽然是目录名，但作为目标使用),如果不声明.PHONY，Make 会认为目标已存在(目录总是存在)且是最新的
# 不会执行清理命令
.PHONY: clean-all $(CLEAN_ALL)
