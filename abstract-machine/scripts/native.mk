AM_SRCS := native/trm.c \
           native/ioe.c \
           native/cte.c \
           native/trap.S \
           native/vme.c \
           native/mpe.c \
           native/platform.c \
           native/ioe/input.c \
           native/ioe/timer.c \
           native/ioe/gpu.c \
           native/ioe/uart.c \
           native/ioe/audio.c \
           native/ioe/disk.c \

# -fpie - Position Independent Executable
# 允许程序加载到内存的任意地址，支持 ASLR（地址空间布局随机化）安全特性
# $(shell sdl2-config --cflags) - SDL2 编译选项
# sdl2-config 工具：SDL2 库提供的配置工具，自动返回正确的编译选项
# 返回-I/usr/include/SDL2 -D_REENTRANT
# -I/usr/include/SDL2 - SDL2 头文件路径
# -D_REENTRANT - 定义宏，启用线程安全的 C 库函数
CFLAGS  += -fpie $(shell sdl2-config --cflags)
# -fpie（汇编器）：生成位置无关的汇编代码,与 C 代码保持一致
ASFLAGS += -fpie -pie
# 定义一个逗号变量, Make 中逗号有特殊含义，直接写会被解析为参数分隔符
comma = ,
# 将链接器选项转换为 g++ 格式
# -Wl,option 的含义: -Wl - "Pass to linker"; ，- 分隔符; option - 链接器选项
# $(addprefix -Wl$(comma), $(LDFLAGS)) - 将 LDFLAGS 中的每个选项添加 -Wl 前缀
# -pie 链接为位置无关可执行文件,与 -fpie 配合使用, 告诉链接器生成 PIE 可执行文件
# -ldl 链接 libdl 库（动态链接库）, Native 平台使用 dlopen() 动态加载 Capstone 反汇编库
# $(shell sdl2-config --libs) 返回 -lSDL2 - 链接 libSDL2.so
LDFLAGS_CXX = $(addprefix -Wl$(comma), $(LDFLAGS)) -pie -ldl $(shell sdl2-config --libs)

# 用户执行：$ make run ARCH=native
# Make 执行：make image 构建 hello-native.elf
#  ./build/hello-native.elf  直接运行
run: image
	$(IMAGE).elf

# -ex command - Execute command :在 GDB 启动时执行命令
# handle signal [keywords] - 设置信号处理方式
# 信号列表：SIGUSR1 - 用户自定义信号 1; SIGUSR2 - 用户自定义信号 2; SIGSEGV - 段错误
# 关键字：noprint - 不打印信号接收消息 ; nostop - 不停止程序
gdb: image
	gdb -ex "handle SIGUSR1 SIGUSR2 SIGSEGV noprint nostop" $(IMAGE).elf
