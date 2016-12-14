CC = gcc
override CFLAGS := -std=c99 -lm -lSDL2 -lSDL2_gfx -fopenmp -O3 $(CFLAGS)

SOURCES = $(wildcard nbody/*.c)
EXECUTABLES = $(SOURCES:.c=.out)

all: $(EXECUTABLES)

clean:
	rm -f $(EXECUTABLES)

%.out: %.c
	$(CC) $< $(CFLAGS) -o $@

.PHONY: all clean
