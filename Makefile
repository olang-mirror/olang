TARGET    := olang
SRC_DIR   := src
BUILD_DIR := build
CFLAGS    := -Werror -Wall -Wextra -Wmissing-declarations -pedantic -std=c11 -ggdb

SRCS      := $(wildcard $(SRC_DIR)/*.c)
HEADERS   := $(wildcard $(SRC_DIR)/*.h)
OBJS      := $(patsubst $(SRC_DIR)/%.c, $(BUILD_DIR)/%.o, $(SRCS))

.PHONY: all
all: $(TARGET)

$(TARGET): $(BUILD_DIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@

.PHONY: format
format: $(SRCS) $(HEADERS)
	clang-format --dry-run --Werror $?
	$(MAKE) -C tests/unit/ format

.PHONY: format-fix
format-fix: $(SRCS) $(HEADERS)
	clang-format -i $?
	$(MAKE) -C tests/unit/ format-fix

.PHONY: integration-test
integration-test:
	$(MAKE)
	$(MAKE) -C tests/integration/

.PHONY: unit-test
unit-test:
	$(MAKE)
	$(MAKE) -C tests/unit/

.PHONY: clean
clean:
	$(MAKE) -C tests/unit/ clean
	@rm -rf build/ $(TARGET)

.PHONY: check
check:
	$(MAKE)
	$(MAKE) -C tests/integration/
	$(MAKE) -C tests/unit/

.PHONY: docs
docs:
	$(MAKE) -C docs

.PHONY: docs-dist
docs-dist:
	$(MAKE) -C docs dist

$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
