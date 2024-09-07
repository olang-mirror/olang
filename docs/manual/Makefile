MAKEINFO=makeinfo

HTML_HEADER=$(shell cat _header.html)

all: html

html: olang.texi introduction.texi installation.texi getting-started.texi contribution-guide.texi
	$(MAKEINFO) --set-customization-variable AFTER_BODY_OPEN="$(HTML_HEADER)" --css-include=../style.css -o html --html olang.texi

.PHONY: clean
clean:
	rm -rf html
