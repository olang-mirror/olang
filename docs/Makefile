PANDOC     := pandoc
INDEX      := index.md
BUILD_DIR  := build
TARGET     := $(BUILD_DIR)/index.html
PAGES_DIR  := pages
PAGES      := $(wildcard $(PAGES_DIR)/*.md)
HTML_PAGES := $(patsubst $(PAGES_DIR)/%.md, $(BUILD_DIR)/$(PAGES_DIR)/%.html, $(PAGES))

.PHONY: all
all:  $(BUILD_DIR) $(TARGET) $(PAGES)

.PHONY: clean
clean:
	rm -rf build/

$(TARGET): $(HTML_PAGES)
	$(PANDOC) -s --template template.html -f markdown -t html $(INDEX) > $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@
	@mkdir -p $@/$(PAGES_DIR)

$(BUILD_DIR)/$(PAGES_DIR)/%.html: $(PAGES_DIR)/%.md
	$(PANDOC) -s --template template.html -f markdown -t html $< > $@
