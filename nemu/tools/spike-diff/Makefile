#***************************************************************************************
# Copyright (c) 2014-2024 Zihao Yu, Nanjing University
#
# NEMU is licensed under Mulan PSL v2.
# You can use this software according to the terms and conditions of the Mulan PSL v2.
# You may obtain a copy of Mulan PSL v2 at:
#          http://license.coscl.org.cn/MulanPSL2
#
# THIS SOFTWARE IS PROVIDED ON AN "AS IS" BASIS, WITHOUT WARRANTIES OF ANY KIND,
# EITHER EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO NON-INFRINGEMENT,
# MERCHANTABILITY OR FIT FOR A PARTICULAR PURPOSE.
#
# See the Mulan PSL v2 for more details.
#**************************************************************************************/

REPO_PATH = repo
ifeq ($(wildcard repo/spike_main),)
  $(shell git clone --depth=1 git@github.com:NJU-ProjectN/riscv-isa-sim $(REPO_PATH))
endif

REPO_BUILD_PATH = $(REPO_PATH)/build
REPO_MAKEFILE = $(REPO_BUILD_PATH)/Makefile
$(REPO_MAKEFILE):
# $(@D) 是 Makefile 自动变量，表示目标的目录部分
	@mkdir -p $(@D)
# abspath 是 Makefile 函数，将相对路径转换为绝对路径
# && 连接命令, 前一个命令成功后才执行后一个
# cd repo/build && /home/jian/ics2025/nemu/tools/spike-diff/repo/configure
# 切换到构建目录,运行 Spike 的 configure 脚本,生成 repo/build/Makefile
	cd $(@D) && $(abspath $(REPO_PATH))/configure
# -i	直接修改文件（in-place）
# -e	指定替换表达式
# 's/-g -O2/-O2/' 替换模式
# 查找：  -g -O2 替换为：-O2
	sed -i -e 's/-g -O2/-O2/' $@

SPIKE = $(REPO_BUILD_PATH)/spike
$(SPIKE): $(REPO_MAKEFILE)
# 设置环境变量 CFLAGS，传递给子 Make
# fvisibility=hidden 的作用：默认隐藏所有符号，只导出显式标记的符号
# $(^D) 表示所有依赖的目录部分
	CFLAGS="-fvisibility=hidden" CXXFLAGS="-fvisibility=hidden" $(MAKE) -C $(^D)

BUILD_DIR = ./build
$(shell mkdir -p $(BUILD_DIR))

# 生成头文件搜索路径
inc_dependencies = fesvr riscv disasm customext fdt softfloat spike_main spike_dasm build
INC_PATH  = -I$(REPO_PATH) $(addprefix -I$(REPO_PATH)/, $(inc_dependencies))
INC_PATH += -I$(NEMU_HOME)/include
# 生成库文件路径
lib_dependencies = libspike_main.a libriscv.a libdisasm.a libsoftfloat.a libfesvr.a libfdt.a
INC_LIBS  = $(addprefix $(REPO_PATH)/build/, $(lib_dependencies))

NAME = $(GUEST_ISA)-spike-so
BINARY = $(BUILD_DIR)/$(NAME)
SRCS = difftest.cc

$(BINARY): $(SPIKE) $(SRCS)
	g++ -std=c++17 -O2 -shared -fPIC -fvisibility=hidden $(INC_PATH) $(SRCS) $(INC_LIBS) -o $@

clean:
	rm -rf $(BUILD_DIR)

all: $(BINARY)
.DEFAULT_GOAL = all

.PHONY: all clean $(SPIKE)
