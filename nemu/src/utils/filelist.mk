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

ifeq ($(CONFIG_ITRACE)$(CONFIG_IQUEUE),)
# $(CONFIG_ITRACE)$(CONFIG_IQUEUE) 将两个配置变量连接成一个字符串
# 如果两个配置都未设置（为空），则条件为真
SRCS-BLACKLIST-y += src/utils/disasm.c
else
# 定义 Capstone 库的路径
LIBCAPSTONE = tools/capstone/repo/libcapstone.so.5
# 添加 Capstone 头文件路径到编译标志
CFLAGS += -I tools/capstone/repo/include
# src/utils/disasm.c 依赖于 $(LIBCAPSTONE)
# 编译 disasm.c 前会先确保库文件存在
src/utils/disasm.c: $(LIBCAPSTONE)
# 如果库文件不存在，执行构建规则
# -C tools/capstone 切换到 capstone 目录执行 make
$(LIBCAPSTONE):
	$(MAKE) -C tools/capstone
endif
