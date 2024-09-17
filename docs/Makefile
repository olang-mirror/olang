PANDOC     := pandoc
INDEX      := index.html
BUILD_DIR  := build
SITE_DIR   := $(BUILD_DIR)/site
TARGET     := $(SITE_DIR)/index.html
DIST_FILE  := site.tar.gz
STYLE      := style.css

.PHONY: all
all:  $(BUILD_DIR) $(STYLE) $(TARGET) manual

.PHONY: clean
clean:
	rm -rf $(BUILD_DIR)
	rm -f $(DIST_FILE)
	make -C info clean

.PHONY: dist
dist: $(DIST_FILE)

.PHONY: manual
manual: $(SITE_DIR)/manual/index.html

$(DIST_FILE): all
	tar -czf $(DIST_FILE) -C $(SITE_DIR) .

$(TARGET): index.html
	@cp $(INDEX) $(TARGET)

$(BUILD_DIR):
	@mkdir -p $@
	@mkdir -p $(SITE_DIR)

$(STYLE): $(BUILD_DIR)
	cp style.css $(SITE_DIR)

$(SITE_DIR)/manual/index.html: info/*.texi
	make -C info
	rm -rf $(SITE_DIR)/manual
	mv info/html $(SITE_DIR)/manual
