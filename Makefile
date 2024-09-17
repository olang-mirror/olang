.POSIX:

CC := gcc

CFLAGS := ${CFLAGS}
CFLAGS += -Werror -Wall -Wextra -Wmissing-declarations
CFLAGS += -pedantic -std=c11 -ggdb

TARGET := olang

PREFIX ?= /usr/local

BINDIR ?= ${PREFIX}/bin
DATADIR ?= ${PREFIX}/share
INFODIR ?= ${DATADIR}/info
MANDIR ?= ${DATADIR}/man
SRCDIR := src
BUILDDIR := build

SRCS := $(wildcard $(SRCDIR)/*.c)
HEADERS := $(wildcard $(SRCDIR)/*.h)
OBJS := $(patsubst $(SRCDIR)/%.c, $(BUILDDIR)/%.o, $(SRCS))

.PHONY: all
all: $(TARGET)

.PHONY: info
info: olang.info

# install target

.PHONY: install
install: install-info

.PHONY: install-info
install-info: olang.info
	install -Dm 644 olang.info ${DESTDIR}${INFODIR}/olang.info
	gzip -f ${DESTDIR}${INFODIR}/olang.info

# uninstall target

.PHONY: uninstall
uninstall: uninstall-info

.PHONY: uninstall-info
uninstall-info:
	@rm -f ${DESTDIR}${INFODIR}/olang.info.gz

olang.info: docs/info/*.texi
	$(MAKEINFO) docs/info/olang.texi

$(TARGET): $(BUILDDIR) $(OBJS)
	$(CC) $(CFLAGS) $(OBJS) -o $(TARGET)

$(BUILDDIR):
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
	@rm -f olang.info
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

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
