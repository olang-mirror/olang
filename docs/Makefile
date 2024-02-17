PANDOC     := pandoc
INDEX      := index.md
BUILD_DIR  := build
TARGET     := $(BUILD_DIR)/index.html
DIST_FILE  := site.tar.gz
PAGES_DIR  := pages
PAGES      := $(wildcard $(PAGES_DIR)/*.md)
HTML_PAGES := $(patsubst $(PAGES_DIR)/%.md, $(BUILD_DIR)/$(PAGES_DIR)/%.html, $(PAGES))

.PHONY: all
all:  $(BUILD_DIR) $(TARGET) $(PAGES)

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(DIST_FILE)

.PHONY: dist
dist: $(DIST_FILE)

$(DIST_FILE): all
	tar -czf $(DIST_FILE) -C $(BUILD_DIR) .

$(TARGET): $(HTML_PAGES)
	$(PANDOC) -s --template template.html -f markdown -t html $(INDEX) > $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@
	@mkdir -p $@/$(PAGES_DIR)

$(BUILD_DIR)/$(PAGES_DIR)/%.html: $(PAGES_DIR)/%.md
	$(PANDOC) -s --template template.html -f markdown -t html $< > $@
