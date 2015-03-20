include $(TOPDIR)/rules.mk

PKG_NAME:=autelan-lib
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

AUTELAN_LIBS=$(PKG_BUILD_DIR)
APPKEY_PATH=etc/appkey

export FILENO_PATH=$(TOPDIR)/fileno/$(TARGET_DIR_NAME)
export FILENO_BIN=$(SCRIPT_DIR)/fileno
export OS_TYPE=openwrt

TARGET_CFLAGS += -Wall -fPIC \
		-fexceptions -fno-omit-frame-pointer \
		-I$(AUTELAN_LIBS) \
		-D__OPENWRT__ \
		-D__TAB_AS_SPACE=4 \
		-std=gnu99
		
TARGET_LDFLAGS+= -shared

define Package/autelan-lib/install/common
	$(INSTALL_DIR) $(1)/lib/
	$(INSTALL_DIR) $(1)/$(APPKEY_PATH)/
endef
####################################################################
define Package/libautelan-appkey
  SECTION:=libs
  CATEGORY:=autelan-lib
  TITLE:=Autelan Basic library
  DEPENDS:=+libubox
endef
	
define Package/libautelan-appkey/install
	$(Package/autelan-lib/install/common)
	
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/appkey/libautelan-appkey.so $(1)/lib/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/appkey/libappkey.limit $(1)/$(APPKEY_PATH)/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/appkey/libappkey.enum $(1)/$(APPKEY_PATH)/
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/appkey/libappkey.key $(1)/$(APPKEY_PATH)/
endef

define Package/libautelan-appkey/compile
	$(MAKE) -C $(PKG_BUILD_DIR)/appkey \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			-D__THIS_NAME=\\\"libappkey\\\" \
			-D__AKID_DEBUG=__libappkey_debug \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
	$(CP) $(PKG_BUILD_DIR)/appkey/libautelan-appkey.so $(STAGING_DIR)/lib
endef
####################################################################
define Build/Prepare
	mkdir -p $(PKG_BUILD_DIR)
	$(CP) ./src/* $(PKG_BUILD_DIR)
	
	mkdir -p $(FILENO_PATH)
endef

define Build/Configure
endef

define Build/Compile
	$(Package/libautelan-appkey/compile)
endef
####################################################################
$(eval $(call BuildPackage,libautelan-appkey))
