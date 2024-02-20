SRCS         := $(wildcard *_test.c)
OBJS         := $(patsubst %_test.c, %_test.o, $(SRCS))
SUBJECT_OBJS := $(filter-out ../../build/0c.o, $(wildcard ../../build/*.o))
CFLAGS       := -I../../src -I../shared
TESTS        := $(patsubst %_test.c, %_test, $(SRCS))
EXEC_TESTS   := $(patsubst %_test, ./%_test, $(TESTS))
MUNIT_SRC    := ../shared/munit.c
MUNIT        := ./munit.o

.PHONY: all
all: $(MUNIT) $(TESTS)
	@for file in $(EXEC_TESTS); do \
                ./"$$file"; \
        done

.PHONY: clean
clean:
	$(RM) *.o *_test
	$(RM) -rfv lib

.PHONY: linter
linter: $(SRCS)
	clang-format --dry-run --Werror $?

.PHONY: linter-fix
linter-fix: $(SRCS)
	clang-format -i $?

%_test: $(MUNIT) $(SUBJECT_OBJS) %_test.c
	$(CC) $? $(CFLAGS) -o $@

$(MUNIT):
	$(CC) -c $(MUNIT_SRC) $(CFLAGS) -o $(MUNIT)
