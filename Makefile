
prefix ?= /usr/local
libdir ?= lib
incdir ?= include

CC ?= gcc

CFLAGS ?= -g -Wall -O2
CFLAGS += -fPIC

LIBS=-lm

LIBNAME=librapl.so
MAJOR=1
MINOR=0
LIBCOMPLETE=$(LIBNAME).$(MAJOR).$(MINOR)

.PHONY: clean tools

all: $(LIBNAME)

$(LIBNAME): rapl.o
	$(CC) $(LDFLAGS) -Wl,-soname,$(LIBNAME).$(MAJOR) -shared $< -o $(LIBCOMPLETE)
	ln -s $(LIBCOMPLETE) $(LIBNAME).$(MAJOR)
	ln -s $(LIBCOMPLETE) $(LIBNAME)

install:
	# install the real library
	install -m 0755 -D $(LIBCOMPLETE) $(DESTDIR)/$(prefix)/$(libdir)/$(LIBCOMPLETE)
	# versioned symlinks
	ln -s $(LIBCOMPLETE) $(DESTDIR)/$(prefix)/$(libdir)/$(LIBNAME)
	ln -s $(LIBCOMPLETE) $(DESTDIR)/$(prefix)/$(libdir)/$(LIBNAME).$(MAJOR)
	
	install -m 0644 -D rapl.h $(DESTDIR)/$(prefix)/$(incdir)/rapl.h
	sed "s/%prefix%/$(subst /,\/,$(prefix))/;\
		s/%libdir%/$(subst /,\/,$(libdir))/; \
		s/%incdir%/$(subst /,\/,$(incdir))/" librapl.pc.skel > librapl.pc
	install -m 0644 -D librapl.pc $(DESTDIR)/$(prefix)/$(libdir)/pkgconfig/librapl.pc

tools:
	$(MAKE) -C tools

clean:
	rm -f $(LIBNAME) $(LIBNAME).$(MAJOR) $(LIBCOMPLETE) *.o
	$(MAKE) -C tools clean