SRCS        := $(wildcard *_test.c)
OBJS        := $(patsubst %_test.c, %_test.o, $(SRCS))
CFLAGS      := -I../../src -I../shared
TESTS       := $(patsubst %_test.c, %_test, $(SRCS))
EXEC_TESTS  := $(patsubst %_test, ./%_test, $(TESTS))
MUNIT_SRC   := ../shared/munit.c
MUNIT       := ./munit.o

.PHONY: all
all: $(MUNIT) proc_exec.o cli_runner.o $(TESTS)
	@for file in $(EXEC_TESTS); do \
                ./"$$file"; \
        done

.PHONY: clean
clean:
	$(RM) *.o *_test

.PHONY: format
format: $(SRCS)
	clang-format --dry-run --Werror $?

.PHONY: format-fix
format-fix: $(SRCS)
	clang-format -i $?

cli_test: $(MUNIT) proc_exec.o cli_runner.o cli_test.o
	$(CC) $? $(CFLAGS) -o $@

$(MUNIT):
	$(CC) -c $(MUNIT_SRC) $(CFLAGS) -o $(MUNIT)
