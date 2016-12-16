CC = gcc
override CFLAGS := -std=c99 -lm -lSDL2 -lSDL2_gfx -fopenmp -O3 $(CFLAGS)

SOURCES = $(wildcard nbody/*.c)
EXECUTABLES = $(SOURCES:.c=.out)
REPORT = report/report.pdf
REPORT_SOURCE = report/report.tex report/report.bib

all: $(EXECUTABLES) $(REPORT)
report: $(REPORT)

clean:
	rm -f $(EXECUTABLES)
	rm -f report/*.log
	rm -f report/*.aux
	rm -f report/*.out
	rm -f report/*.toc
	rm -f report/*.pdf
	rm -f report/*.bbl
	rm -f report/*.blg

%.out: %.c
	$(CC) $< $(CFLAGS) -o $@

$(REPORT): $(REPORT_SOURCE) $(REPORT_IMAGES)
	pdflatex --output-directory=report $<
	export BIBINPUTS=report && bibtex $(basename $<)
	pdflatex --output-directory=report $<
	pdflatex --output-directory=report $<

.PHONY: all clean report
