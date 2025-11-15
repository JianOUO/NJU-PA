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

-include $(NEMU_HOME)/../Makefile
include $(NEMU_HOME)/scripts/build.mk

include $(NEMU_HOME)/tools/difftest.mk

# compile_git：伪目标，不生成实际文件
# $(call git_commit, "compile NEMU")：调用之前定义的 git_commit 函数(在ics2025/Makefile中定义)
# 作用：在编译时自动创建一个 Git 提交，记录编译事件
compile_git:
	$(call git_commit, "compile NEMU")
# $(BINARY)：最终的可执行文件目标
# compile_git：前置条件
# 双冒号允许一个目标有多个规则
# 在make nemu时，会同时执行compile_git和$(BINARY)的构建规则
# compiler_git用于追踪编译事件，构建规则用于生成可执行文件目标
$(BINARY):: compile_git

# Some convenient rules
# override：覆盖指令，确保这个赋值不会被命令行参数覆盖
# 当没有override时,运行make run ARGS="--help"
# ARGS 被命令行覆盖，结果为: --help，丢失了默认参数和追加参数！
# 有override时，make run ARGS="--help" 
# 结果为: --help $(EXTRA_ARGS): 用户参数 + 强制追加参数
# ?=：条件赋值，如果未定义则赋值
override ARGS ?= --log=$(BUILD_DIR)/nemu-log.txt
override ARGS += $(ARGS_DIFF)

# Command to execute NEMU
# IMG：磁盘镜像文件参数
# ?=：条件赋值，如果未定义则为空
# 作用：允许用户指定要加载的磁盘镜像文件
IMG ?=
# $(BINARY)：NEMU 可执行文件（如 nemu 或 riscv32-nemu-interpreter）
# $(ARGS)：运行参数（包括日志、差分测试等）
# $(IMG)：磁盘镜像文件
# 作用：组合成完整的执行命令
NEMU_EXEC := $(BINARY) $(ARGS) $(IMG)

#run-env：运行环境准备目标
# $(BINARY)：依赖NEMU可执行文件
# $(DIFF_REF_SO)：依赖差分测试参考实现
# 作用：确保运行前所有必要的组件都已构建
run-env: $(BINARY) $(DIFF_REF_SO)

# 依赖：run-env（确保环境就绪）
# Git 提交：记录运行事件
# 执行：运行 NEMU 模拟器
run: run-env
	$(call git_commit, "run NEMU")
	$(NEMU_EXEC)

gdb: run-env
	$(call git_commit, "gdb NEMU")
# -s：GDB 选项，指定符号文件
# $(BINARY)：NEMU 可执行文件路径
# 作用：让 GDB 加载可执行文件的调试符号
# --args：GDB 选项，传递参数给被调试程序
# $(NEMU_EXEC)：完整的 NEMU 执行命令
	gdb -s $(BINARY) --args $(NEMU_EXEC)

# find ./tools -maxdepth 2 -mindepth 2 -name "Makefile" 
# -maxdepth 2	最多向下搜索 2 层
# -mindepth 2	最少向下搜索 2 层
# 结果：
# ./tools/kconfig/Makefile
# ./tools/capstone/Makefile
# ./tools/gen-expr/Makefile
# ./tools/qemu-diff/Makefile
# ./tools/kvm-diff/Makefile
# ./tools/fixdep/Makefile
# ./tools/spike-diff/Makefile
# dir：提取目录路径
clean-tools = $(dir $(shell find ./tools -maxdepth 2 -mindepth 2 -name "Makefile"))
# 这是一个模式规则，展开为：
#./tools/kconfig/:
#	-@$(MAKE) -s -C ./tools/kconfig/ clean
#./tools/fixdep/:
#	-@$(MAKE) -s -C ./tools/fixdep/ clean
#./tools/spike-diff/:
#	-@$(MAKE) -s -C ./tools/spike-diff/ clean
# ...
# -	忽略错误（如果清理失败继续执行），@	静默执行（不显示命令），$(MAKE)	make 程序本身
# -s silent，减少输出，-C $@ 切换到目标目录，clean 执行 clean 目标
$(clean-tools):
	-@$(MAKE) -s -C $@ clean
# clean-tools 目标依赖所有工具目录,当执行 make clean-tools 时，会依次清理每个工具。
clean-tools: $(clean-tools)
# clean	清理编译生成的文件（.o, .d, 可执行文件）
# distclean	清理配置文件（.config, auto.conf 等）
# clean-tools	清理所有工具的构建结果
clean-all: clean distclean clean-tools

.PHONY: run gdb run-env clean-tools clean-all $(clean-tools)
