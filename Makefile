PLATFORMS = aix ansi bsd freebsd generic linux macosx mingw posix solaris

ifndef PLATFORM
   $(error Please set PLATFORM to one of the following: $(PLATFORMS))
endif

ifndef PREFIX
   PREFIX = .
endif

build:
	make -C lua $(PLATFORM)
	make -C git
	make -C miconf

clean:
	make -C lua clean
	make -C git clean
	make -C miconf clean

install: build
	install -d $(PREFIX)/bin
	install miconf/miconf $(PREFIX)/bin
	install git/git-find-large $(PREFIX)/bin
	install git/git-generate-version-info $(PREFIX)/bin
	install git/git-update-timestamp $(PREFIX)/bin
