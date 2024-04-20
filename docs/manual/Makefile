MAKEINFO=makeinfo

all: html

html: olang.texi introduction.texi installation.texi getting-started.texi contribution-guide.texi
	$(MAKEINFO) --css-include=style.css -o html --html olang.texi

.PHONY: clean
clean:
	rm -rf html
