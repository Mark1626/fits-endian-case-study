CFLAGS=-Wall -Wextra -O3
DEBUG_FLAGS=-fsanitize=address -g
LIBS=`pkg-config --libs cfitsio`
OMP_FLAGS=-fopenmp

ifdef DEBUG
  CFLAGS+=$(DEBUG_FLAGS)
endif

ifdef OMP
	CFLAGS+=$(OMP_FLAGS)
endif

all: directories build/imstat build/imstat_opt build/imstat_fread

directories:
	mkdir -p build

build/%: src/%.c
	$(CC) -o $@ $< $(CFLAGS) $(LIBS)

clean:
	rm build/*

.PHONY: directories clean
