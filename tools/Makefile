
CC ?= gcc

CFLAGS ?= -g -Wall -O2
CFLAGS += -I../
LIBS = -L../ -lrapl

APP=rapl_dump

all: $(APP)

$(APP): $(APP).o
	$(CC) $(LDFLAGS) $< -o $@ $(LIBS)

clean:
	rm -f $(APP) *.o

run:
	LD_LIBRARY_PATH=../ ./$(APP)

gnuplot:
	LD_LIBRARY_PATH=../ ./$(APP) 0 rapldata
	gnuplot rapldata.gnuplot