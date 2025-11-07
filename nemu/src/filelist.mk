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

SRCS-y += src/nemu-main.c
DIRS-y += src/cpu src/monitor src/utils
DIRS-$(CONFIG_MODE_SYSTEM) += src/memory
DIRS-BLACKLIST-$(CONFIG_TARGET_AM) += src/monitor/sdb

SHARE = $(if $(CONFIG_TARGET_SHARE),1,0)
# -lreadline
# 作用：链接 readline 库
# 功能：提供命令行编辑和历史记录功能
# 用途：在 NEMU 中用于实现交互式命令行界面
# 效果：用户可以使用方向键、Tab 补全、历史命令等功能
# -ldl
# 作用：链接动态链接库（Dynamic Linking Library）
# 功能：提供动态加载和链接共享库的功能
# 用途：支持 dlopen()、dlsym() 等动态链接函数
# 效果：允许程序在运行时加载和使用共享库
# -pie
# 作用：生成位置无关可执行文件（Position Independent Executable）
# 功能：使可执行文件可以在内存中的任意位置加载
# 用途：提高安全性，支持地址空间布局随机化（ASLR）
# 效果：每次运行时程序在内存中的位置可能不同
LIBS += $(if $(CONFIG_TARGET_NATIVE_ELF),-lreadline -ldl -pie,)

# mainargs只要通过命令行传递，如 make mainargs=/path/to/some/binary
# 当 mainargs 被设置为某个路径时：汇编器会定义宏 BIN_PATH；
# am-bin.S 中的 .incbin BIN_PATH 会将该路径的二进制文件直接嵌入到编译后的 NEMU 可执行文件中
# 程序运行时可以通过 bin_start 和 bin_end 符号访问这段数据
ifdef mainargs
# ASFLAGS 是汇编器标志变量
# -D 定义预处理器宏
# BIN_PATH=\"$(mainargs)\" 定义宏 BIN_PATH，值为 mainargs 变量的内容
# \" 是转义的双引号，确保在宏定义中正确包含引号
ASFLAGS += -DBIN_PATH=\"$(mainargs)\"
endif
SRCS-$(CONFIG_TARGET_AM) += src/am-bin.S
# 如果当前目录下存在名为 src/am-bin.S 的实际文件
# 没有 .PHONY 声明时，Make 会认为目标已经是最新的，不会执行相关规则
# 有了 .PHONY 声明，Make 会忽略同名文件的存在，总是执行相关规则
.PHONY: src/am-bin.S
