PREFIX ?= .
PLATFORM ?= $(shell scripts/miconf-platform)

INSTALL = install -p

LUA_PLATFORM = $(PLATFORM)
SYSLIBS = 
ifeq ($(PLATFORM),Linux)
   LUA_PLATFORM = linux
   SYSLIBS = -lm -Wl,-E -ldl
endif
ifeq ($(PLATFORM),Darwin)
   LUA_PLATFORM = macosx
endif


build:
	$(MAKE) -C lua $(LUA_PLATFORM)
	$(MAKE) -C scripts
	$(MAKE) -C miconf "SYSLIBS=$(SYSLIBS)"

clean:
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
