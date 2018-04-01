piconf: piconf.c version.h
	gcc -O2 -o $@ $< -lpython27 $(SYSLIBS)

version.h: ../.git
	../scripts/git-generate-version-info miconf c >$@

piconf.html: piconf.md
	multimarkdown piconf.md >piconf.html

clean:
	rm -f piconf version.h
