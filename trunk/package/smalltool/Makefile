#
# Copyright (C) 2007-2013 OpenWrt.org
# Copyright (C) 2010 Vertical Communications
#
# This is free software, licensed under the GNU General Public License v2.
# See /LICENSE for more information.
#

include $(TOPDIR)/rules.mk
include $(INCLUDE_DIR)/version.mk
include $(INCLUDE_DIR)/kernel.mk

PKG_NAME:=smalltool
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

define Package/smalltool
  SECTION:=app
  CATEGORY:=application
  SUBMENU:=app
  DEPENDS:=+libpthread
  TITLE:=some small tools
endef

define Package/smalltool/description
	These are some small tools 
endef
define Build/Prepare
	rm -rf $(PKG_BUILD_DIR)
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./* $(PKG_BUILD_DIR)/
endef

define Build/Compile
	$(MAKE) -C $(PKG_BUILD_DIR)/src CC="$(TARGET_CC)"
endef

define Package/smalltool/install
	$(INSTALL_DIR) $(1)/usr/sbin
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/src/loopcheck $(1)/usr/sbin
endef

$(eval $(call BuildPackage,smalltool))
