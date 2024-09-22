.POSIX:

CC := gcc

CFLAGS := ${CFLAGS}
CFLAGS += -Werror -Wall -Wextra -Wmissing-declarations
CFLAGS += -pedantic -std=c11 -ggdb

TARGET := olc

PREFIX ?= /usr/local

BINDIR ?= ${PREFIX}/bin
DATADIR ?= ${PREFIX}/share
INFODIR ?= ${DATADIR}/info
MANDIR ?= ${DATADIR}/man
MAN1DIR ?= ${MANDIR}/man1
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
install: install-bin install-man install-info

.PHONY: install-bin
install-bin: $(TARGET)
	install -Dm755 $< ${DESTDIR}${BINDIR}/$<

.PHONY: install-man
install-man: install-man1

.PHONY: install-man1
install-man1: docs/man/man1/olc.1
	install -Dm 644 docs/man/man1/olc.1 ${DESTDIR}${MAN1DIR}/olc.1
	gzip -f ${DESTDIR}${MAN1DIR}/olc.1

.PHONY: install-info
install-info: olang.info
	install -Dm 644 olang.info ${DESTDIR}${INFODIR}/olang.info
	gzip -f ${DESTDIR}${INFODIR}/olang.info

# uninstall target

.PHONY: uninstall
uninstall: uninstall-bin uninstall-man uninstall-info

.PHONY: uninstall-bin
uninstall-bin: ${DESTDIR}${BINDIR}/$(TARGET)
	@rm -f ${DESTDIR}${BINDIR}/$(TARGET)

.PHONY: uninstall-man
uninstall-man: uninstall-man1

.PHONY: uninstall-man1
uninstall-man1:
	@rm -f ${DESTDIR}${MAN1DIR}/olc.1.gz

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

.PHONY: check-olc
check-olc: $(TARGET)
	$(MAKE) -C tests/olc/

.PHONY: check-unit
check-unit: $(TARGET)
	$(MAKE) -C tests/unit/

.PHONY: clean
clean:
	@rm -f olang.info
	$(MAKE) -C tests/unit/ clean
	@rm -rf build/ $(TARGET)

.PHONY: check
check: check-unit check-olc

.PHONY: docs
docs:
	$(MAKE) -C docs

.PHONY: docs-dist
docs-dist:
	$(MAKE) -C docs dist

$(BUILDDIR)/%.o: $(SRCDIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@
