
prefix ?= /usr/local
libdir ?= lib
incdir ?= include

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
	install -m 0644 -D rapl.h $(DESTDIR)/$(prefix)/$(incdir)/rapl.h
	sed "s/%prefix%/$(subst /,\/,$(prefix))/;\
		s/%libdir%/$(subst /,\/,$(libdir))/; \
		s/%incdir%/$(subst /,\/,$(incdir))/" librapl.pc.skel > librapl.pc
	install -m 0644 -D librapl.pc $(DESTDIR)/$(prefix)/$(libdir)/pkgconfig/librapl.pc

tools:
	$(MAKE) -C tools

clean:
	rm -f $(LIBNAME) *.o
	$(MAKE) -C tools clean