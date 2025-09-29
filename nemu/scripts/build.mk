# .DEFAULT_GOAL：Makefile 的特殊变量，指定当直接运行 make 时的默认目标
# app：默认构建应用程序（而不是库或其他目标）
# 效果：运行 make 等价于运行 make app
.DEFAULT_GOAL = app

# Add necessary options if the target is a shared library
# SHARE：控制变量，决定构建类型
# SHARE=1：构建共享库（动态链接库）
# SHARE=0 或未设置：构建静态应用程序
ifeq ($(SHARE),1)
# SO：后缀变量，用于生成文件名
# 作用：当构建共享库时，在文件名后添加 -so 后缀
# 示例：myapp → myapp-so（共享库版本）
SO = -so
# -fPIC：生成位置无关代码（Position Independent Code）
# 作用：代码不依赖固定的内存地址
# 为什么需要 PIC？
# 应用程序：加载到固定地址，可以使用绝对地址
# 共享库：可能被多个程序同时使用，加载地址不固定
# -fvisibility=hidden：默认隐藏符号
# 作用：只导出显式标记为可见的函数
# 好处：减少符号冲突，提高加载速度
# 默认所有符号隐藏
# void internal_func() { }  // 不导出，无冲突
# 显式标记需要导出的函数
# __attribute__((visibility("default"))) void public_api() { }
CFLAGS  += -fPIC -fvisibility=hidden
# -shared：生成共享库（而不是可执行文件）
# -fPIC：链接时也需要位置无关代码
LDFLAGS += -shared -fPIC
endif

WORK_DIR  = $(shell pwd)
BUILD_DIR = $(WORK_DIR)/build
# := 赋值时立即展开，只展开一次
# = 延迟展开，每次使用时重新展开
INC_PATH := $(WORK_DIR)/include $(INC_PATH)
OBJ_DIR  = $(BUILD_DIR)/obj-$(NAME)$(SO)
BINARY   = $(BUILD_DIR)/$(NAME)$(SO)

# Compilation flags
# 作用：根据 C 编译器的选择自动设置对应的 C++ 编译器
# 逻辑：如果 CC=clang，则 CXX=clang++，否则 CXX=g++
ifeq ($(CC),clang)
CXX := clang++
else
CXX := g++
endif
# 作用：使用 C++ 编译器作为链接器
# 原因：C++ 编译器能正确处理 C++ 的链接（名称修饰、静态初始化等）
LD := $(CXX)
# INC_PATH：包含目录列表（如 src include lib）
# addprefix：为每个目录添加 -I 前缀
# 示例：如果 INC_PATH = src include，则 INCLUDES = -Isrc -Iinclude
INCLUDES = $(addprefix -I, $(INC_PATH))
# 作用：设置编译和链接选项
# -O2 优化级别: 标准优化
# 效果：在编译时间和代码性能之间取得平衡
# -MMD 告诉 GCC 在编译时自动生成一个 .d 文件,这个 .d 文件包含了源文件的所有依赖关系（头文件等）
# 当源文件包含的头文件发生变化时，Make 能够知道需要重新编译,避免手动维护复杂的依赖关系
# -Wall：启用所有警告
# -Werror：将警告视为错误
CFLAGS  := -O2 -MMD -Wall -Werror $(INCLUDES) $(CFLAGS)
LDFLAGS := -O2 $(LDFLAGS)

# 模式替换语法 $(VARIABLE:PATTERN=REPLACEMENT)
# 作用：将变量中匹配模式的部分替换为指定内容
# %.c：匹配任何以 .c 结尾的字符串
# $(OBJ_DIR)/%.o：替换为在对象目录下的对应 .o 文件
OBJS = $(SRCS:%.c=$(OBJ_DIR)/%.o) $(CXXSRC:%.cc=$(OBJ_DIR)/%.o)

# Compilation patterns
# 目标模式：$(OBJ_DIR)/%.o - 匹配任何在 OBJ_DIR 目录下的 .o 文件
# 依赖模式：%.c - 匹配对应的 .c 源文件
# @：不显示执行的命令本身，只显示输出
$(OBJ_DIR)/%.o: %.c
# + CC：显示正在编译 C 文件
# $<：自动化变量，表示第一个依赖文件（即 .c 文件）
	@echo + CC $<
# mkdir -p：递归创建目录，如果已存在也不报错
# $(dir $@)：获取目标文件的目录路径
# $@：自动化变量，表示目标文件（即 .o 文件）
	@mkdir -p $(dir $@)
	@$(CC) $(CFLAGS) -c -o $@ $<
# $(@:.o=.d)：将目标文件的 .o 替换为 .d（依赖文件）
# call_fixdep：自定义函数，处理依赖关系
# 这行代码的作用是：获取当前目标文件（通常是 .o 文件）将其扩展名从 .o 改为 .d,然后调用 call_fixdep 函数，传入两个参数：第一个参数：依赖文件名（.d 文件）第二个参数：目标文件名（.o 文件） 
# 调用 call_fixdep 函数，传入两个参数：第一个参数：依赖文件名（.d 文件）第二个参数：目标文件名（.o 文件） 
	$(call call_fixdep, $(@:.o=.d), $@)

$(OBJ_DIR)/%.o: %.cc
	@echo + CXX $<
	@mkdir -p $(dir $@)
	@$(CXX) $(CFLAGS) $(CXXFLAGS) -c -o $@ $<
	$(call call_fixdep, $(@:.o=.d), $@)

# Depencies
# -include：如果文件不存在，不报错，继续执行
-include $(OBJS:.o=.d)

# Some convenient rules

# 声明 app 和 clean 是伪目标（phony targets）
# 伪目标不对应实际的文件，而是表示要执行的操作
# 即使存在名为 app 或 clean 的文件，Make 也会执行这些目标
.PHONY: app clean

# 定义 app 目标依赖于 $(BINARY) 变量
# 当执行 make app 时，会先构建 $(BINARY) 所代表的文件
app: $(BINARY)

# :: 表示这是一个双冒号规则
# 允许在同一个 Makefile 中有多个构建 $(BINARY) 的规则
# 每个规则可以处理不同的依赖或执行不同的操作
# 适用于需要多个步骤构建同一个目标的情况
# 工作流程:
# 执行 make app
# Make 检查 $(BINARY) 的依赖关系
# 如果 $(OBJS) 或 $(ARCHIVES) 有更新，执行链接命令
# 生成最终的可执行文件
$(BINARY):: $(OBJS) $(ARCHIVES)
	@echo + LD $@
	@$(LD) -o $@ $(OBJS) $(LDFLAGS) $(ARCHIVES) $(LIBS)

clean:
	-rm -rf $(BUILD_DIR)
