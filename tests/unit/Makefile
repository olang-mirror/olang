SRCS := $(wildcard *.c)
DEP_OBJS := $(filter-out ../../build/main.o, $(wildcard ../../build/*.o))
CFLAGS := -I../../src -I../shared
TESTS := $(patsubst %.c, %.bin, $(SRCS))
RUN_TESTS := $(patsubst %.bin, %.run, $(TESTS))
MUNIT_SRC := ../shared/munit.c
MUNIT := ./munit.o

.PHONY: all clean format format-fix
all: $(RUN_TESTS)

%.bin: %.c $(MUNIT)
	@$(CC) $(CFLAGS) $(MUNIT) $(DEP_OBJS) $< -o $@
	@printf 'CCLD\t%s\n' '$@'

%.run: %.bin
	@./$<

clean:
	@$(RM) *.o *.bin
	@$(RM) -rfv lib

format: $(SRCS)
	@clang-format --dry-run --Werror $?

format-fix: $(SRCS)
	@clang-format -i $?

$(MUNIT):
	@$(CC) -c $(MUNIT_SRC) $(CFLAGS) -o $(MUNIT)
	@printf 'CCLD\t%s\n' '$@'
