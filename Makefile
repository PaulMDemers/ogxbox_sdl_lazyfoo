LAZYFOO_APPS = \
	001_hello_sdl \
	002_image_on_screen \
	003_event_driven_programming \
	004_key_presses \
	005_optimized_surface_loading \
	006_extension_libraries \
	007_texture_loading_rendering \
	008_geometry_rendering \
	009_viewport \
	010_color_keying \
	011_clip_rendering_sprite_sheets \
	012_color_modulation \
	013_alpha_blending \
	014_animated_sprites_vsync \
	015_rotation_flipping

.PHONY: all clean release print-apps $(LAZYFOO_APPS)

all: $(LAZYFOO_APPS)

$(LAZYFOO_APPS):
	$(MAKE) -C $@

clean:
	@for app in $(LAZYFOO_APPS); do $(MAKE) -C $$app clean || exit $$?; done

release: all
	./tools/collect_release_isos.sh

print-apps:
	@printf '%s\n' $(LAZYFOO_APPS)
