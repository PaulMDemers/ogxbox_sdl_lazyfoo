XBE_TITLE = xblazyfoo_006_extension_libraries
GEN_XISO = $(XBE_TITLE).iso
SRCS = $(CURDIR)/main.c
NXDK_DIR ?= $(CURDIR)/../../.nxdk
NXDK_SDL = y

include $(NXDK_DIR)/Makefile

TARGET += $(OUTPUT_DIR)/lazyfoo_006_image.png

$(GEN_XISO): $(OUTPUT_DIR)/lazyfoo_006_image.png

$(OUTPUT_DIR)/lazyfoo_006_image.png: $(CURDIR)/lazyfoo_006_image.png $(OUTPUT_DIR)
	$(VE)cp '$<' '$@'
