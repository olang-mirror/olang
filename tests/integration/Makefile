TESTER_SRC := ./test.sh
TESTS := $(wildcard ./tests/*.ol)

.PHONY: all
all:
	@set -e; \
	for file in $(TESTS); do \
                $(TESTER_SRC) "$$file"; \
        done
