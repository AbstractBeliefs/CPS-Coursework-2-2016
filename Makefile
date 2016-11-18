CC = gcc
override CFLAGS := -std=c99 -lm -fopenmp -O3 $(CXXFLAGS)

SOURCES = $(wildcard nbody/*.c)
EXECUTABLES = $(SOURCES:.c=.out)

all: $(EXECUTABLES)

clean:
	rm -f $(EXECUTABLES)

%.out: %.c
	$(CC) $< $(CFLAGS) -o $@

.PHONY: all clean
