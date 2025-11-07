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

ifdef CONFIG_DIFFTEST
DIFF_REF_PATH = $(NEMU_HOME)/$(call remove_quote,$(CONFIG_DIFFTEST_REF_PATH))
DIFF_REF_SO = $(DIFF_REF_PATH)/build/$(GUEST_ISA)-$(call remove_quote,$(CONFIG_DIFFTEST_REF_NAME))-so
MKFLAGS = GUEST_ISA=$(GUEST_ISA) SHARE=1 ENGINE=interpreter
ARGS_DIFF = --diff=$(DIFF_REF_SO)

# 不以NEMU为参考实现时
ifndef CONFIG_DIFFTEST_REF_NEMU
# -C 切换到指定目录
$(DIFF_REF_SO):
	$(MAKE) -s -C $(DIFF_REF_PATH) $(MKFLAGS)
endif

.PHONY: $(DIFF_REF_SO)
endif

# 首次启用差分测试: make run
# 检查 $(DIFF_REF_SO) 是否存在
# 如果不存在，触发构建规则, 如make -s -C /nemu/tools/spike-diff GUEST_ISA=riscv32 SHARE=1 ENGINE=interpreter
# 生成$(DIFF_REF_SO)
# 然后运行 NEMU 并传递 $ARGS_DIFF: --diff=$(DIFF_REF_SO) 参数(native.mk:40)