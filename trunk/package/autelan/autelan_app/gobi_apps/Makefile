include $(TOPDIR)/rules.mk

PKG_NAME:=gobi_apps
PKG_VERSION:=2014-08-06
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

define Package/gobi_apps
  SECTION:=base
  CATEGORY:=Autelan Apps
  TITLE:=gps/qmuxd/netconn/atc for DM11-1
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
	$(MAKE) $(MAKE_OPTS) LTEFI_V2=1
endef

define Package/gobi_apps/install
	$(INSTALL_DIR) $(1)/etc
	$(CP) $(PKG_BUILD_DIR)/apn_config $(1)/etc
	$(INSTALL_DIR) $(1)/usr/sbin
	$(CP) $(PKG_BUILD_DIR)/gps $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/qmuxd $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/netconn $(1)/usr/sbin/
	$(CP) $(PKG_BUILD_DIR)/atc $(1)/usr/sbin/
	$(INSTALL_DIR) $(1)/lib
	$(CP) $(PKG_BUILD_DIR)/install/lib/libqcci_legacy.so.1 $(1)/lib
	$(CP) $(PKG_BUILD_DIR)/install/lib/libqmi.so.1 $(1)/lib
	$(CP) $(PKG_BUILD_DIR)/install/lib/libqmi_client_qmux.so.1 $(1)/lib
	$(CP) $(PKG_BUILD_DIR)/install/lib/libqmiidl.so.1 $(1)/lib
	$(CP) $(PKG_BUILD_DIR)/install/lib/libqmiservices.so.1 $(1)/lib	
endef

$(eval $(call BuildPackage,gobi_apps))

