
CC ?= gcc

EXTRA_CFLAGS=-fPIC
CFLAGS += $(EXTRA_CFLAGS)

LIBS=-lm

LIBNAME=librapl.so

all: $(LIBNAME)

$(LIBNAME): rapl.o
	$(CC) $(LDFLAGS) -shared $< -o $@ $(LIBS)

tools:
	cd tools
	$(MAKE) $(MFLAGS)

clean:
	rm -f $(LIBNAME) *.o