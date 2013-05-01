
prefix ?= /usr/local
libdir ?= lib

CC ?= gcc

CFLAGS ?= -g -Wall -O2
CFLAGS += -fPIC

LIBS=-lm

LIBNAME=librapl.so

.PHONY: clean tools

all: $(LIBNAME)

$(LIBNAME): rapl.o
	$(CC) $(LDFLAGS) -shared $< -o $@ $(LIBS)

install:
	install -m 0755 -D $(LIBNAME) $(DESTDIR)/$(prefix)/$(libdir)/$(LIBNAME)

tools:
	$(MAKE) -C tools

clean:
	rm -f $(LIBNAME) *.o
	$(MAKE) -C tools clean