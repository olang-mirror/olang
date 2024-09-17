PANDOC     := pandoc
INDEX      := index.html
BUILD_DIR  := build
SITE_DIR   := $(BUILD_DIR)/site
TARGET     := $(SITE_DIR)/index.html
DIST_FILE  := site.tar.gz
STYLE      := style.css
MANPAGES   := $(BUILD_DIR)/man

.PHONY: all
all:  $(BUILD_DIR) $(STYLE) $(TARGET) manpages manual

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(DIST_FILE)
	make -C info clean

.PHONY: dist
dist: $(DIST_FILE)

.PHONY: manpages
manpages: $(BUILD_DIR) $(MANPAGES)/olang.1

.PHONY: manual
manual: $(SITE_DIR)/manual/index.html


$(MANPAGES)/%.1: manpages/%.1
	@cp $< $@

$(DIST_FILE): all
	tar -czf $(DIST_FILE) -C $(SITE_DIR) .

$(TARGET): index.html
	@cp $(INDEX) $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@
	@mkdir -p $(SITE_DIR)
	@mkdir -p $(MANPAGES)

$(STYLE): $(BUILD_DIR)
	cp style.css $(SITE_DIR)

$(SITE_DIR)/manual/index.html: info/*.texi
	make -C info
	rm -rf $(SITE_DIR)/manual
	mv info/html $(SITE_DIR)/manual
