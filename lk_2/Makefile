DIR_SRC         := src
DIR_INCLUDE     := include
DIR_BUILD       := build
DIR_MISC        := misc
DIR_SYSTEMD     := /etc/systemd/system
DIR_LIB         := /usr/lib
DIR_HEADERS     := /usr/include
DIR_SUDO        := /usr/sbin
DIR_BIN         := /usr/bin

APT             := /usr/bin/apt
DPKG_SCAN       := /usr/bin/dpkg-scanpackages
DPKG_DEB        := /usr/bin/dpkg-deb
GZIP            := /usr/bin/gzip

PKG_NAME        := mysyslog
PKG_VERSION     := 1.0.0
PKG_ARCH        := amd64
APT_REPO        := /opt/$(PKG_NAME).apt
DIR_DEBIAN_LIB  := $(DIR_BUILD)/lib$(PKG_NAME)_$(PKG_VERSION)_$(PKG_ARCH)
DIR_DEBIAN_CL   := $(DIR_BUILD)/$(PKG_NAME)-client_$(PKG_VERSION)_$(PKG_ARCH)
DIR_DEBIAN_D    := $(DIR_BUILD)/$(PKG_NAME)d_$(PKG_VERSION)_$(PKG_ARCH)
DIR_DEBIAN_META := $(DIR_BUILD)/$(PKG_NAME)-meta_$(PKG_VERSION)_$(PKG_ARCH)

CCFLAGS         := -I$(DIR_INCLUDE) -DUSE_BACKEND_MYSYSLOG
CLIENT_SRC      := $(DIR_SRC)/mysyslog_client.c
CLIENT_OUT      := $(DIR_BUILD)/mysyslog-client
DAEMON_SRC      := $(DIR_SRC)/mysyslog_daemon.c
DAEMON_OUT      := $(DIR_BUILD)/mysyslogd
MYSYSLOG_SRC    := $(DIR_SRC)/mysyslog.c
MYSYSLOG_OUT    := $(DIR_BUILD)/drivers/libmysyslog.so

DRIVERS_CCFLAGS := -I$(DIR_INCLUDE) -fPIC -DUSE_BACKEND_MYSYSLOG -shared
DRIVERS_SRCS    := $(wildcard $(DIR_SRC)/drivers/*.c)
DRIVERS_SO      := $(patsubst $(DIR_SRC)/drivers/%.c, $(DIR_BUILD)/drivers/lib%.so, $(DRIVERS_SRCS))


all: create_dirs drivers libmysyslog deb client daemon

install:
	@mkdir -p $(APT_REPO)
	@cp $(DIR_BUILD)/*.deb $(APT_REPO)
	@cd $(APT_REPO) && $(DPKG_SCAN) . /dev/null | $(GZIP) -9c > $(APT_REPO)/Packages.gz
	@rm -rf /etc/apt/sources.list.d/$(PKG_NAME).list
	@echo "deb [trusted=yes] file://$(APT_REPO) ./" >> /etc/apt/sources.list.d/$(PKG_NAME).list
	$(APT) update
	$(APT) install $(PKG_NAME)-meta

uninstall:
	@rm -rf /etc/apt/sources.list.d/$(PKG_NAME).list
	@rm -rf $(APT_REPO)
	$(APT) autoremove $(PKG_NAME)-meta

debug: CCFLAGS += -DDEBUG -ggdb
debug: DRIVERS_CCFLAGS += -DDEBUG -ggdb
debug: all

client: libmysyslog
	@cc $(CCFLAGS) -o $(CLIENT_OUT) $(CLIENT_SRC) -L$(DIR_BUILD)/drivers -lmysyslog

daemon: libmysyslog
	@cc $(CCFLAGS) -o $(DAEMON_OUT) $(DAEMON_SRC) -L$(DIR_BUILD)/drivers -lmysyslog

deb: dependencies mysyslog-meta_deb

dependencies: libmysyslog_deb mysyslogd_deb mysyslog-client_deb

mysyslog-meta_deb: dependencies
	@mkdir -p $(DIR_DEBIAN_META)
	@cp -r $(DIR_MISC)/$(PKG_NAME)-meta/* $(DIR_DEBIAN_META)
	$(DPKG_DEB) --root-owner-group --build $(DIR_DEBIAN_META)

mysyslog-client_deb: libmysyslog_deb client
	@mkdir -p $(DIR_DEBIAN_CL)$(DIR_BIN)
	@cp -r $(DIR_MISC)/$(PKG_NAME)-client/* $(DIR_DEBIAN_CL)
	@cp $(DIR_BUILD)/$(PKG_NAME)-client $(DIR_DEBIAN_CL)$(DIR_BIN)
	$(DPKG_DEB) --root-owner-group --build $(DIR_DEBIAN_CL)

mysyslogd_deb: libmysyslog_deb daemon
	@mkdir -p $(DIR_DEBIAN_D)$(DIR_SUDO)
	@cp -r $(DIR_MISC)/$(PKG_NAME)d/* $(DIR_DEBIAN_D)
	@cp $(DIR_BUILD)/$(PKG_NAME)d $(DIR_DEBIAN_D)$(DIR_SUDO)
	$(DPKG_DEB) --root-owner-group --build $(DIR_DEBIAN_D)

libmysyslog_deb: create_dirs drivers libmysyslog
	@mkdir -p $(DIR_DEBIAN_LIB)/$(DIR_LIB)
	@mkdir -p $(DIR_DEBIAN_LIB)/$(DIR_HEADERS)
	@cp -r $(DIR_MISC)/lib$(PKG_NAME)/* $(DIR_DEBIAN_LIB)
	@cp $(DIR_BUILD)/drivers/*.so $(DIR_DEBIAN_LIB)/$(DIR_LIB)
	@cp $(DIR_INCLUDE)/mysyslog.h $(DIR_DEBIAN_LIB)/$(DIR_HEADERS)
	@cp $(DIR_INCLUDE)/mysyslog_back.h $(DIR_DEBIAN_LIB)/$(DIR_HEADERS)
	$(DPKG_DEB) --root-owner-group --build $(DIR_DEBIAN_LIB)

libmysyslog: create_dirs
	@cc $(DRIVERS_CCFLAGS) -o $(MYSYSLOG_OUT) $(MYSYSLOG_SRC)

drivers: $(DRIVERS_SO)

$(DIR_BUILD)/drivers/lib%.so: $(DIR_SRC)/drivers/%.c
	@cc $(DRIVERS_CCFLAGS) -o $@ $<

create_dirs:
	@mkdir -p $(DIR_BUILD)/drivers
	@mkdir -p $(DIR_DEBIAN_LIB)

clean:
	@rm -rf $(DIR_BUILD)
