SRCS        := $(wildcard *_test.c)
OBJS        := $(patsubst %_test.c, %_test.o, $(SRCS))
CFLAGS      := -I../../src -I../shared
TESTS       := $(patsubst %_test.c, %_test, $(SRCS))
EXEC_TESTS  := $(patsubst %_test, ./%_test, $(TESTS))
MUNIT_SRC   := ../shared/munit.c
MUNIT       := ./munit.o

.PHONY: all
all: $(MUNIT) $(TESTS)
	@for file in $(EXEC_TESTS); do \
                ./"$$file"; \
        done

.PHONY: clean
clean:
	$(RM) *.o *_test

.PHONY: linter
linter: $(SRCS)
	clang-format --dry-run --Werror $?

.PHONY: linter-fix
linter-fix: $(SRCS)
	clang-format -i $?

$(MUNIT):
	$(CC) -c $(MUNIT_SRC) $(CFLAGS) -o $(MUNIT)
