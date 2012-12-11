all: fs formatfs

fs: fs.c
	gcc -o $@ -I/usr/pkg/include -L/usr/pkg/lib $< -lreadline -lncurses

formatfs: formatfs.c
	gcc -o $@ $<



