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

# echo "\033[1;31m"：输出 ANSI 转义序列
# \033[1;31m：
# \033[：转义序列开始
# 1：粗体/高亮度
# 31：红色前景
# m：转义序列结束
COLOR_RED := $(shell echo "\033[1;31m")
# \033[0m：重置所有文本属性（恢复正常颜色）
COLOR_END := $(shell echo "\033[0m")

ifeq ($(wildcard .config),)
# $(warning) 会在 Makefile 解析阶段（而不是执行阶段）显示警告信息，但不会停止构建过程。
$(warning $(COLOR_RED)Warning: .config does not exist!$(COLOR_END))
$(warning $(COLOR_RED)To build the project, first run 'make menuconfig'.$(COLOR_END))
endif

# Q：命令控制变量（通常代表 "Quiet"）
# @：在 Makefile 中表示不显示执行的命令本身
# 作用：通过在命令前使用 $(Q) 来控制是否显示命令
# $(Q)echo "Compiling..."    # 如果 Q=@，只显示 "Compiling..."
# $(Q)gcc -c file.c -o file.o # 不显示 gcc 命令本身
Q            := @
KCONFIG_PATH := $(NEMU_HOME)/tools/kconfig
FIXDEP_PATH  := $(NEMU_HOME)/tools/fixdep
Kconfig      := $(NEMU_HOME)/Kconfig
# rm-distclean：用于 make distclean 目标时要删除的文件列表
rm-distclean += include/generated include/config .config .config.old
silent := -s

# $(CONF):：目标文件，当 $(KCONFIG_PATH)/build/conf 不存在或过时时触发
# $(Q)：静默执行（@，不显示命令本身）
# $(MAKE)：Make 程序本身（通常是 make）
# $(silent)：静默标志（-s，减少输出）
# -C $(KCONFIG_PATH)：切换到 KCONFIG_PATH 目录执行 Make
# NAME=conf：传递给子 Make 的参数，指定构建目标
CONF   := $(KCONFIG_PATH)/build/conf
MCONF  := $(KCONFIG_PATH)/build/mconf
FIXDEP := $(FIXDEP_PATH)/build/fixdep

$(CONF):
	$(Q)$(MAKE) $(silent) -C $(KCONFIG_PATH) NAME=conf

$(MCONF):
	$(Q)$(MAKE) $(silent) -C $(KCONFIG_PATH) NAME=mconf

$(FIXDEP):
	$(Q)$(MAKE) $(silent) -C $(FIXDEP_PATH)

# 确保运行 menuconfig 前，所有必要的工具都已经编译好。
menuconfig: $(MCONF) $(CONF) $(FIXDEP)
# $(Q)：静默执行（@，不显示命令本身）
# $(MCONF)：菜单配置工具（tools/kconfig/build/mconf）
# $(Kconfig)：主配置文件（Kconfig）
# 作用：启动图形化配置界面，让用户选择配置选项。
	$(Q)$(MCONF) $(Kconfig)
# $(CONF)：配置解析器（tools/kconfig/build/conf）
# $(silent)：静默标志（-s，减少输出）
# --syncconfig：同步配置选项
#作用：将用户在界面中的选择转换为构建系统可用的文件。
	$(Q)$(CONF) $(silent) --syncconfig $(Kconfig)

# 等价于tools/kconfig/build/conf --savedefconfig=configs/defconfig Kconfig
# 作用：将最小化的默认配置保存到 configs/defconfig 文件中。
savedefconfig: $(CONF)
	$(Q)$< $(silent) --$@=configs/defconfig $(Kconfig)

# %defconfig：模式规则，匹配任何以 defconfig 结尾的目标
# $(CONF) $(FIXDEP)：依赖的工具
# --defconfig=configs/$@：从指定文件加载默认配置
# --syncconfig：同步配置到构建系统
%defconfig: $(CONF) $(FIXDEP)
# 从 configs/%_defconfig 文件加载预设配置到 .config
	$(Q)$< $(silent) --defconfig=configs/$@ $(Kconfig)
# 处理配置依赖，生成构建系统需要的文件
	$(Q)$< $(silent) --syncconfig $(Kconfig)

.PHONY: menuconfig savedefconfig defconfig

# Help text used by make help
help:
	@echo  '  menuconfig	  - Update current config utilising a menu based program'
	@echo  '  savedefconfig   - Save current config as configs/defconfig (minimal config)'

distclean: clean
	-@rm -rf $(rm-distclean)

.PHONY: help distclean

# $(1)：第一个参数（通常是依赖文件 .d）
# $(2)：第二个参数（通常是目标文件 .o）
# unused：固定参数，fixdep 工具要求的格式
# > $(1).tmp：将输出重定向到临时文件
# 当使用 gcc -MMD 时，生成的依赖文件包含：
# 原始生成的 build/src/main.d
# build/src/main.o: src/main.c /usr/include/stdio.h \
  /usr/include/features.h /usr/include/stdc-predef.h \
  include/common.h include/config.h
# ... 很多系统头文件
# fixdep 处理后的 build/src/main.d
# build/src/main.o: src/main.c include/common.h include/config.h
# fixdep 的功能
# 过滤系统头文件：移除 /usr/include/ 等系统路径
# 保留项目文件：只保留项目相关的头文件
# 优化增量编译：避免系统头文件修改触发不必要的重新编译
# tools/fixdep/build/fixdep build/src/main.d build/src/main.o unused > build/src/main.d.tmp
# bulid/src/main.d 是原始的依赖文件，build/src/main.d.tmp 是处理后的依赖文件
# mv build/src/main.d.tmp build/src/main.d
# 用处理后的依赖文件替换原始依赖文件
define call_fixdep
	@$(FIXDEP) $(1) $(2) unused > $(1).tmp
	@mv $(1).tmp $(1)
endef
