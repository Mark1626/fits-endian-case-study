CFLAGS=-Wall -Wextra -O3
DEBUG_FLAGS=-fsanitize=address -g
LIBS=`pkg-config --cflags --libs cfitsio`
OMP_FLAGS=-fopenmp

ifdef DEBUG
  CFLAGS+=$(DEBUG_FLAGS)
endif

ifdef OMP
	CFLAGS+=$(OMP_FLAGS)
endif

all: directories build/imstat build/imstat_opt build/imstat_fread build/imcvt_bit_to_little build/imstat_fread_le

directories:
	mkdir -p build

build/%.o: src/%.c src/common.h
	$(CC) -o $@ -c $< $(CFLAGS) $(LIBS)

build/%: src/%.c build/common.o
	$(CC) -o $@ $< build/common.o $(CFLAGS) $(LIBS)

clean:
	rm build/*

.PHONY: directories clean
