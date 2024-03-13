PANDOC     := pandoc
INDEX      := index.md
BUILD_DIR  := build
SITE_DIR   := $(BUILD_DIR)/site
TARGET     := $(SITE_DIR)/index.html
DIST_FILE  := site.tar.gz
PAGES_DIR  := pages
MANPAGES   := $(BUILD_DIR)/man
PAGES      := $(wildcard $(PAGES_DIR)/*.md)
HTML_PAGES := $(patsubst $(PAGES_DIR)/%.md, $(SITE_DIR)/$(PAGES_DIR)/%.html, $(PAGES))

.PHONY: all
all:  $(BUILD_DIR) $(TARGET) $(PAGES) manpages

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(DIST_FILE)

.PHONY: dist
dist: $(DIST_FILE)

.PHONY: manpages
manpages: $(BUILD_DIR) $(MANPAGES)/olang.1

$(MANPAGES)/%.1: manpages/%.md
	$(PANDOC) -s -t man $< > $@

$(DIST_FILE): all
	tar -czf $(DIST_FILE) -C $(SITE_DIR) .

$(TARGET): $(HTML_PAGES)
	$(PANDOC) -s --template template.html -f markdown -t html $(INDEX) > $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@
	@mkdir -p $(SITE_DIR)/$(PAGES_DIR)
	@mkdir -p $(MANPAGES)

$(SITE_DIR)/$(PAGES_DIR)/%.html: $(PAGES_DIR)/%.md
	$(PANDOC) -s --template template.html -f markdown -t html --toc $< > $@
