miconf: miconf.c version.h
	gcc -O2 -I../lua/src -L../lua/src -o $@ $< -l lua

version.h: ../.git
	../git/git-generate-version-info miconf c >$@

miconf.html: miconf.md
	multimarkdown miconf.md >miconf.html

clean:
	rm -f miconf version.h
