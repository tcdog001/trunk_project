include $(TOPDIR)/rules.mk

PKG_NAME:=rgps
PKG_VERSION:=2014-08-07
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/rgps
  SECTION:=base
  CATEGORY:=Autelan Apps
  TITLE:=resolve gps
  DEPENDS:= +libpthread
endef

MAKE_OPTS:= -C "$(PKG_BUILD_DIR)" \
	CROSS="$(KERNEL_CROSS)" \
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

define Package/rgps/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/rgps $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/* $(1)/sbin
endef

$(eval $(call BuildPackage,rgps))

