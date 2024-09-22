.POSIX:

RUNNER := ./run.sh
SRCS := $(wildcard *.ol)
TESTS := $(patsubst %.ol, %.run, $(SRCS))

.SUFFIXES:
.SUFFIXES: .ol .run

.PHONY: all
all: $(TESTS)

.ol.run:
	@$(RUNNER) ./$<

