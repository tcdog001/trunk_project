include $(TOPDIR)/rules.mk

PKG_NAME:=at_ctrl
PKG_VERSION:=2014-07-17
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/at_ctrl
  SECTION:=base
  CATEGORY:=Autelan Apps
  TITLE:=at command read&write ttyUSB1(MC2716) ttyUSB3(C5300)
  DEPENDS:= +libpthread
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
	$(MAKE) $(MAKE_OPTS) LTEFI_V2=1
endef

define Package/at_ctrl/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/at_ttyUSB1 $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/at_ttyUSB3 $(1)/usr/sbin/
endef

$(eval $(call BuildPackage,at_ctrl))

