include $(TOPDIR)/rules.mk

PKG_NAME:=acc_module
PKG_VERSION:=2014-07-25
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define KernelPackage/acc_module
  SECTION:=base
  CATEGORY:=Autelan Modules
  TITLE:=acc_mod.ko
  FILES:=$(PKG_BUILD_DIR)/acc_mod.ko
  AUTOLOAD:=$(call AutoProbe,acc_mod)
endef

MAKE_OPTS:= -C "$(PKG_BUILD_DIR)" \
	CROSS_COMPILE="$(KERNEL_CROSS)" \
	ARCH="$(LINUX_KARCH)" \
	EXTRA_CFLAGS="-I$(PKG_BUILD_DIR)/include" \
	KLIB_BUILD="$(LINUX_DIR)" \
	MODPROBE=true \
	KLIB=$(TARGET_MODULES_DIR) \
	KERNEL_SUBLEVEL=$(lastword $(subst ., ,$(KERNEL_PATCHVER))) \
	EXTRA_CFLAGS="$(EXTRA_CFLAGS)" \
	KBUILD_LDFLAGS_MODULE_PREREQ=
#	CC=$(TARGET_CC) \
#	KERNELDIR="$(KERNEL_BUILD_DIR)/linux-3.10.21"

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(MAKE_OPTS) LTEFI_V2=1
endef

define KernelPackage/acc_module/install
	#$(INSTALL_DIR) $(1)/lib/modules/3.10.21
	#$(CP) $(PKG_BUILD_DIR)/acc_mod.ko $(1)/lib/modules/3.10.21
endef

$(eval $(call KernelPackage,acc_module))

