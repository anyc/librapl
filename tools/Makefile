
CC ?= gcc

CFLAGS += -I../
LDFLAGS += -L../ -lrapl

APP=rapl_dump

all: $(APP)

$(APP): $(APP).o
	$(CC) $< -o $@ $(LDFLAGS)

clean:
	rm -f $(APP) *.o

run:
	LD_LIBRARY_PATH=../ ./$(APP)

gnuplot:
	LD_LIBRARY_PATH=../ ./$(APP) 0 rapldata
	gnuplot rapldata.gnuplot