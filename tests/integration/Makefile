SRCS        := $(wildcard *_test.c)
TO_LINT     := $(filter-out munit.c munit.h,$(wildcard *.c *.h))
OBJS        := $(patsubst %_test.c, %_test.o, $(SRCS))
CFLAGS      := -I../../src
TESTS       := $(patsubst %_test.c, %_test, $(SRCS))
EXEC_TESTS  := $(patsubst %_test, ./%_test, $(TESTS))

.PHONY: all
all: munit.o cli_runner.o $(TESTS)
	@for file in $(EXEC_TESTS); do \
                ./"$$file"; \
        done

.PHONY: clean
clean:
	$(RM) *.o *_test

.PHONY: linter
linter: $(TO_LINT)
	clang-format --dry-run --Werror $?

.PHONY: linter-fix
linter-fix: $(TO_LINT)
	clang-format -i $?

cli_test: munit.o cli_runner.o cli_test.o
	$(CC) $? $(CFLAGS) -o $@
