all: a.out hidden/liba.dylib hidden/libb.dylib

CC ?= clang
ifeq "$(ARCH)" ""
	ARCH = x86_64
endif
CFLAGS ?=-arch $(ARCH) -gdwarf-2 -O0
CWD := $(shell pwd)

a.out: main.o hidden/liba.dylib
	$(CC) $(CFLAGS) -o $@ $< -Lhidden -la

hidden/liba.dylib: a.o
	$(CC) $(CFLAGS) -dynamiclib -o $@ $^
	dsymutil $@

hidden/libb.dylib: a.o
	$(CC) $(CFLAGS) -dynamiclib -o $@ $^
	dsymutil $@

clean:
	rm -rf a.out hidden/* *.o
