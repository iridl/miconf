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

clean distclean:
	$(MAKE) -C lua clean
	$(MAKE) -C scripts clean
	$(MAKE) -C miconf clean

install: build
	$(INSTALL) -d $(PREFIX)/bin
	$(INSTALL) miconf/miconf $(PREFIX)/bin
	$(INSTALL) scripts/miconf-platform $(PREFIX)/bin
	$(INSTALL) scripts/git-find-large $(PREFIX)/bin
	$(INSTALL) scripts/git-generate-version-info $(PREFIX)/bin
	$(INSTALL) scripts/git-update-timestamp $(PREFIX)/bin
