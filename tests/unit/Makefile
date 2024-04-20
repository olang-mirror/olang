SRCS         := $(wildcard *_test.c)
OBJS         := $(patsubst %_test.c, %_test.o, $(SRCS))
SUBJECT_OBJS := $(filter-out ../../build/main.o, $(wildcard ../../build/*.o))
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

.PHONY: format
format: $(SRCS)
	clang-format --dry-run --Werror $?

.PHONY: format-fix
format-fix: $(SRCS)
	clang-format -i $?

%_test: $(MUNIT) $(SUBJECT_OBJS) %_test.c
	$(CC) $? $(CFLAGS) -o $@

$(MUNIT):
	$(CC) -c $(MUNIT_SRC) $(CFLAGS) -o $(MUNIT)
