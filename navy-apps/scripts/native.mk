# 定义 native 模式的编译和运行规则，使 Navy-apps 程序能在本地 Linux 上运行，通过 LD_PRELOAD 模拟设备文件
# 使用 C++ 编译器作为链接器, 因为C++ 需要处理名称修饰(name mangling)，需要处理静态初始化, 需要链接 C++ 标准库
LD = $(CXX)

### Run an application with $(ISA)=native

# 准备运行环境，构建 native.so 共享库（用于模拟设备文件
env:
	$(MAKE) -C $(NAVY_HOME)/libs/libos ISA=native

# LD_PRELOAD=...：在程序启动前加载指定的共享库, 可以拦截系统调用(如 open、read、write), 用于模拟设备文件(如 /dev/fb、/dev/events)
run: app env
	@LD_PRELOAD=$(NAVY_HOME)/libs/libos/build/native.so $(APP) $(mainargs)

GDB_ENV = -ex "set environment XDG_RUNTIME_DIR /mnt/wslg/runtime-dir" \
 -ex "set environment LD_PRELOAD $(NAVY_HOME)/libs/libos/build/native.so"

# -ex "set environment LD_PRELOAD ..."：在 GDB 中设置环境变量
# --args $(APP) $(mainargs)：指定要调试的程序和参数
gdb: app env
#	@gdb -ex "set environment LD_PRELOAD $(NAVY_HOME)/libs/libos/build/native.so" --args $(APP) $(mainargs)
	@gdb $(GDB_ENV) --args $(APP) $(mainargs)
.PHONY: env run gdb
