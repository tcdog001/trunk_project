include $(TOPDIR)/rules.mk

PKG_NAME:=gobi_driver_140321
PKG_VERSION:=2014-07-25
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
#PKG_CONFIG_DEPENDS := CONFIG_PM
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define KernelPackage/gobi_driver_140321
  SECTION:=base
  CATEGORY:=Autelan Modules
  TITLE:=GobiSerial.ko
  DEPENDS:=+kmod-usb-serial-wwan +kmod-usb-wdm +kmod-usb-net
  FILES:=$(PKG_BUILD_DIR)/GobiSerial.ko 
  AUTOLOAD:=$(call AutoProbe,GobiSerial)
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
#	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(MAKE_OPTS)
endef

define KernelPackage/gobi_driver_140321/install
#	$(INSTALL_DIR) $(1)/lib/modules/3.10.21
#	$(CP) $(PKG_BUILD_DIR)/GobiSerial.ko $(1)/lib/modules/3.10.21
endef

$(eval $(call KernelPackage,gobi_driver_140321))

