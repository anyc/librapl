
prefix ?= /usr/local
bindir ?= bin

CC ?= gcc

CFLAGS ?= -g -Wall -O2
CFLAGS += -I../
LIBS = -L../ -lrapl -lm

APP=rapl_dump

all: $(APP)

$(APP): $(APP).o
	$(CC) $(LDFLAGS) $< -o $@ $(LIBS)

clean:
	rm -f $(APP) *.o

install:
	install -m 0755 -D $(APP) $(DESTDIR)/$(prefix)/$(bindir)/$(APP)

run:
	LD_LIBRARY_PATH=../ ./$(APP)

gnuplot:
	LD_LIBRARY_PATH=../ ./$(APP) 0 rapldata
	gnuplot rapldata.gnuplot