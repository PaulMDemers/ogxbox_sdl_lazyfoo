XBE_TITLE = xblazyfoo_051_sdl_modern_opengl_nxgl
GEN_XISO = $(XBE_TITLE).iso
NXDK_DIR ?= $(CURDIR)/../../.nxdk
NXGL_DIR ?= $(CURDIR)/../../nxgl

include $(NXGL_DIR)/nxgl.mk

SRCS = $(NXGL_SRCS) $(CURDIR)/main.c
SHADER_OBJS = $(NXGL_SHADER_OBJS)
CFLAGS += $(NXGL_CFLAGS)

include $(NXDK_DIR)/Makefile
