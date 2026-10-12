REPO_PATH = repo
CONFIG_FILE = config/nanos-lite.config
PATCH_FILE = $(abspath patch/diff.patch)
ifeq ($(wildcard repo/include/busybox.h),)
  $(shell git clone --depth=1 -b 1_32_stable git://git.busybox.net/busybox $(REPO_PATH))
  $(shell cd $(REPO_PATH) && git apply $(PATCH_FILE))
  $(shell cp $(CONFIG_FILE) $(REPO_PATH)/.config)
endif

NAME = busybox
include $(NAVY_HOME)/Makefile

ifeq ($(ISA), native)
CFLAGS += -I$(WORK_DIR)/include/native/
else
CFLAGS += -I$(WORK_DIR)/include/navy/ -D__GLIBC__ -fdata-sections -ffunction-sections -nostdlib
comma = ,
LDFLAGS := $(addprefix -Wl$(comma), $(LDFLAGS))
export LDFLAGS
endif

export CFLAGS
export CROSS_COMPILE

BUSYBOX_MAKE_VARS = CFLAGS="$(CFLAGS)" LDFLAGS="$(LDFLAGS)" CROSS_COMPILE="$(CROSS_COMPILE)" LINKAGE="$(LINKAGE)"

# The busybox Makefile can not correctly handle the dependency when
# $(LINKAGE) are updated. If it is the case, perform a fresh build.
$(APP): force;
	$(MAKE) -C $(REPO_PATH) $(BUSYBOX_MAKE_VARS)
	cp $(REPO_PATH)/busybox $@

force: ;

menuconfig:
	$(MAKE) -C $(REPO_PATH) $@

install:
	$(MAKE) -C $(REPO_PATH) CONFIG_PREFIX=$(NAVY_HOME)/fsimg $(BUSYBOX_MAKE_VARS) $@

clean: clean-busybox

clean-busybox:
	$(MAKE) -C $(REPO_PATH) clean

.PHONY: force menuconfig clean-busybox
