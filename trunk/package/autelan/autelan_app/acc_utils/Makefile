include $(TOPDIR)/rules.mk

PKG_NAME:=acc_utils
PKG_VERSION:=2014-07-17
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)-$(PKG_VERSION)
include $(INCLUDE_DIR)/package.mk
include $(INCLUDE_DIR)/kernel.mk

FILES:= acc_setget acc_monitor

define PartGen
define Package/$(1)
	TITLE:=$(1) package from acc_utils
	SECTION:=base
	CATEGORY:=Autelan Apps
	SUBMENU:=acc_utils
	DEPENDS:=+libpthread +libuci +libnl-tiny +libubus +ubus +ubusd +jshn +kmod-acc_module
	MAINTAINER:=AUTELAN
endef
endef

$(foreach file,$(FILES),$(eval $(call PartGen,$(file))))

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

define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) src/* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) $(MAKE_OPTS) LTEFI_V3=1
endef

define PartInstall
define Package/$(1)/install
	$(INSTALL_DIR) $$(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/$(1) $$(1)/usr/sbin
	$(INSTALL_DIR) $$(1)/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/scripts/* $$(1)/sbin
	$(INSTALL_DIR) $$(1)/etc/init.d
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/init.d/autelan $$(1)/etc/init.d
	$(INSTALL_DIR) $$(1)/etc/config
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/config/autelan $$(1)/etc/config
endef
endef


$(foreach file,$(FILES),$(eval $(call PartInstall,$(file))))
$(foreach file,$(FILES),$(eval $(call BuildPackage,$(file))))
