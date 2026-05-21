XBE_TITLE = xblazyfoo_032_text_input_clipboard
GEN_XISO = $(XBE_TITLE).iso
SRCS = $(CURDIR)/main.c
NXDK_DIR ?= $(CURDIR)/../../.nxdk
NXDK_SDL = y

all_local: copy_font all

include $(NXDK_DIR)/Makefile

copy_font:
	@mkdir -p $(OUTPUT_DIR)
	cp $(NXDK_DIR)/samples/sdl_ttf/vegur-regular.ttf $(OUTPUT_DIR)/
