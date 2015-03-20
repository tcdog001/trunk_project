include $(TOPDIR)/rules.mk

PKG_NAME:=autelan-app
PKG_RELEASE:=1

PKG_BUILD_DIR := $(BUILD_DIR)/$(PKG_NAME)

include $(INCLUDE_DIR)/package.mk

AUTELAN_LIBS=$(BUILD_DIR)/autelan-lib

export FILENO_PATH=$(TOPDIR)/fileno/$(TARGET_DIR_NAME)
export FILENO_BIN=$(SCRIPT_DIR)/fileno
export OS_TYPE=openwrt

TARGET_CFLAGS += -Wall \
		-fexceptions -fno-omit-frame-pointer \
		-I$(AUTELAN_LIBS) \
		-D__OPENWRT__ \
		-D__DEBUG_INIT \
		-D__TAB_AS_SPACE=4 \
		-std=gnu99
		
#TARGET_LDFLAGS+= -L$(STAGING_DIR)/lib -Wl,-rpath,$(STAGING_DIR)/lib
TARGET_LDFLAGS+= -L$(STAGING_DIR)/lib

APPKEY_PATH=etc/appkey
BACKTRACE_PATH=usr/app/backtrace

define Package/autelan-app/install/common
	$(INSTALL_DIR) $(1)/usr/bin/
	$(INSTALL_DIR) $(1)/etc/config/
	$(INSTALL_DIR) $(1)/etc/init.d/
	$(INSTALL_DIR) $(1)/$(BACKTRACE_PATH)/
	$(INSTALL_DIR) $(1)/$(APPKEY_PATH)/
endef
####################################################################
define Package/autelan-appkey
  SECTION:=apps
  CATEGORY:=autelan-app
  TITLE:=Autelan Basic App
  DEPENDS:=+libubacktrace +libautelan-appkey +libubox
endef

define Package/autelan-appkey/install
	$(Package/autelan-app/install/common)
	$(INSTALL_DIR) $(1)/$(BACKTRACE_PATH)/appkey/
	
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/appkey/appkey $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/appkey/appkey.key $(1)/$(APPKEY_PATH)/
endef

define Package/autelan-appkey/compile
	$(MAKE) -C $(PKG_BUILD_DIR)/appkey \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			-D__THIS_NAME=\\\"appkey\\\" \
			-D__AKID_DEBUG=__appkey_debug \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Package/autelan-btt
  SECTION:=apps
  CATEGORY:=autelan-app
  TITLE:=Autelan Basic App
  DEPENDS:=+libubacktrace +libautelan-appkey +libubox
endef

define Package/autelan-btt/install
	$(Package/autelan-app/install/common)
	$(INSTALL_DIR) $(1)/$(BACKTRACE_PATH)/btt/
	
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/btt/btt $(1)/usr/bin
endef

define Package/autelan-btt/compile
	$(MAKE) -C $(PKG_BUILD_DIR)/btt \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			-D__THIS_NAME=\\\"btt\\\" \
			-D__AKID_DEBUG=__btt_debug \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Package/autelan-partool
  SECTION:=apps
  CATEGORY:=autelan-app
  TITLE:=Autelan Basic App
  DEPENDS:=+libubacktrace +libautelan-appkey +libubox
endef

define Package/autelan-partool/install
	$(Package/autelan-app/install/common)
	$(INSTALL_DIR) $(1)/$(BACKTRACE_PATH)/partool/
	
	$(INSTALL_BIN) $(PKG_BUILD_DIR)/partool/partool $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/partool/partool.key $(1)/$(APPKEY_PATH)/
endef

define Package/autelan-partool/compile
	$(MAKE) -C $(PKG_BUILD_DIR)/partool \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			-D__THIS_NAME=\\\"partool\\\" \
			-D__AKID_DEBUG=__partool_debug \
			-DNAME_PRODUCT=\\\"mtd7\\\" \
			-DNAME_OSENV=\\\"mtd2\\\" \
			-DPART_RW_MTD \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Package/autelan-um
  SECTION:=apps
  CATEGORY:=autelan-app
  TITLE:=Autelan Basic App
  DEPENDS:=+libubacktrace +libautelan-appkey +libuci +libubus +libubox +libblobmsg-json
endef

define Package/autelan-um/install
	$(Package/autelan-app/install/common)
	$(INSTALL_DIR) $(1)/$(BACKTRACE_PATH)/um/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/um/um $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/um/um.key $(1)/$(APPKEY_PATH)/
	cp -f $(PKG_BUILD_DIR)/um/um.conf $(1)/etc/config/um
	cp -f $(PKG_BUILD_DIR)/um/um.init $(1)/etc/init.d/um
endef

define Package/autelan-um/compile
	$(MAKE) -C $(PKG_BUILD_DIR)/um \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			-D__THIS_NAME=\\\"um\\\" \
			-D__AKID_DEBUG=__um_debug \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Package/autelan-stimerd
  SECTION:=apps
  CATEGORY:=autelan-app
  TITLE:=Autelan Basic App
  DEPENDS:=+libubacktrace +libautelan-appkey
endef

define Package/autelan-stimerd/install
	$(Package/autelan-app/install/common)
	$(INSTALL_DIR) $(1)/$(BACKTRACE_PATH)/stimerd/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/stimerd/stimerd $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/stimerd/stimerd.key $(1)/$(APPKEY_PATH)/
endef

define Package/autelan-stimerd/compile
	$(MAKE) -C $(PKG_BUILD_DIR)/stimerd \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			-D__THIS_NAME=\\\"stimerd\\\" \
			-D__AKID_DEBUG=__stimerd_debug \
			-DSTIMER_TICKS=1000 \
			-DSTIMER_TIMEOUT_TICKS=5000 \
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
endef
####################################################################
define Package/autelan-stimerc
  SECTION:=apps
  CATEGORY:=autelan-app
  TITLE:=Autelan Basic App
  DEPENDS:=+libubacktrace +libautelan-appkey
endef

define Package/autelan-stimerc/install
	$(Package/autelan-app/install/common)
	$(INSTALL_DIR) $(1)/$(BACKTRACE_PATH)/stimerc/

	$(INSTALL_BIN) $(PKG_BUILD_DIR)/stimerc/stimerc $(1)/usr/bin
	$(INSTALL_DATA) $(PKG_BUILD_DIR)/stimerc/stimerc.key $(1)/$(APPKEY_PATH)/
endef

define Package/autelan-stimerc/compile
	$(MAKE) -C $(PKG_BUILD_DIR)/stimerc \
		CC="$(TARGET_CC)" \
		CFLAGS=" \
			$(TARGET_CFLAGS) \
			$(TARGET_CPPFLAGS) \
			-D__THIS_NAME=\\\"stimerc\\\" \
			-D__AKID_DEBUG=__stimerc_debug \
			-D__USE_INLINE_TIMER \
			-DSTIMER_TICKS=1000 \
			-DSTIMER_TIMEOUT_TICKS=10000
			" \
		LDFLAGS="$(TARGET_LDFLAGS)"
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
	$(Package/autelan-btt/compile)
	$(Package/autelan-appkey/compile)
	$(Package/autelan-partool/compile)
	$(Package/autelan-um/compile)
	$(Package/autelan-stimerd/compile)
	$(Package/autelan-stimerc/compile)
endef
####################################################################
$(eval $(call BuildPackage,autelan-btt))
$(eval $(call BuildPackage,autelan-appkey))
$(eval $(call BuildPackage,autelan-partool))
$(eval $(call BuildPackage,autelan-um))
$(eval $(call BuildPackage,autelan-stimerd))
$(eval $(call BuildPackage,autelan-stimerc))
