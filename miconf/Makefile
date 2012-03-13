miconf: miconf.c
	echo "#define MICONF_RELEASE \"`git describe --tags --exact-match  --match 'miconf-[0-9.]*'`\""  >version
	gcc -O2 -I../lua/src -L../lua/src -o miconf miconf.c -l lua

clean:
	rm -f miconf version
