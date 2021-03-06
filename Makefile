PREFIX ?= .
PLAT ?= $(shell scripts/miconf-platform)

INSTALL = install

LUA_PLAT = posix
SYSLIBS = 
ifeq ($(PLAT),Linux)
   LUA_PLAT = linux
   SYSLIBS = -lm -Wl,-E -ldl
endif
ifeq ($(PLAT),Darwin)
   LUA_PLAT = macosx
endif


build:
	$(MAKE) -C lua $(LUA_PLAT)
	$(MAKE) -C scripts
	$(MAKE) -C miconf "SYSLIBS=$(SYSLIBS)"
	$(MAKE) -C piconf "SYSLIBS=$(SYSLIBS)"

clean distclean:
	$(MAKE) -C lua clean
	$(MAKE) -C scripts clean
	$(MAKE) -C miconf clean
	$(MAKE) -C piconf clean

install: build
	$(INSTALL) -d $(PREFIX)/bin
	$(INSTALL) miconf/miconf $(PREFIX)/bin
	$(INSTALL) piconf/piconf $(PREFIX)/bin
	$(INSTALL) lua/src/luac $(PREFIX)/bin
	$(INSTALL) lua/src/lua $(PREFIX)/bin
	$(INSTALL) scripts/miconf-platform $(PREFIX)/bin
	$(INSTALL) scripts/miconf-multi $(PREFIX)/bin
	$(INSTALL) scripts/miconf-traverse $(PREFIX)/bin
	$(INSTALL) scripts/miconf-rstparser $(PREFIX)/bin
	$(INSTALL) scripts/git-find-large $(PREFIX)/bin
	$(INSTALL) scripts/git-generate-version-info $(PREFIX)/bin
	$(INSTALL) scripts/git-update-timestamp $(PREFIX)/bin
	$(INSTALL) scripts/git-backup-repositories $(PREFIX)/bin
	$(INSTALL) scripts/bitbucket-backup $(PREFIX)/bin
	$(INSTALL) scripts/git-release-tarball $(PREFIX)/bin
	$(INSTALL) scripts/scan.pl $(PREFIX)/bin

