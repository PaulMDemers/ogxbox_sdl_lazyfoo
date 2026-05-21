NXDK_DIR ?= $(CURDIR)/../.nxdk
NXGL_DIR ?= $(CURDIR)/../nxgl

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
	015_rotation_flipping \
	016_true_type_fonts \
	017_mouse_events \
	018_key_states \
	019_gamepads_joysticks \
	020_force_feedback \
	022_timing \
	023_advanced_timers \
	024_calculating_frame_rate \
	025_capping_frame_rate \
	026_motion \
	027_collision_detection \
	028_per_pixel_collision \
	029_circular_collision \
	030_scrolling \
	031_scrolling_backgrounds \
	032_text_input_clipboard \
	033_file_reading_writing \
	035_window_events \
	036_multiple_windows \
	037_multiple_displays \
	038_particle_engines \
	039_tiling \
	040_texture_manipulation \
	041_bitmap_fonts \
	042_texture_streaming \
	043_render_to_texture \
	044_frame_independent_movement \
	045_timer_callbacks \
	046_multithreading \
	047_semaphores \
	048_atomic_operations \
	049_mutexes_conditions

OPENGL_VARIANTS = \
	050_sdl_opengl_2_pbkit \
	051_sdl_modern_opengl_pbkit

ifneq ($(wildcard $(NXGL_DIR)/nxgl.mk),)
OPENGL_VARIANTS := \
	050_sdl_opengl_2_nxgl \
	050_sdl_opengl_2_pbkit \
	051_sdl_modern_opengl_nxgl \
	051_sdl_modern_opengl_pbkit
endif

LAZYFOO_APPS += $(OPENGL_VARIANTS)

MIXER_INSERT_AFTER = \
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
	015_rotation_flipping \
	016_true_type_fonts \
	017_mouse_events \
	018_key_states \
	019_gamepads_joysticks \
	020_force_feedback

ifneq ($(wildcard $(NXDK_DIR)/lib/sdl/SDL_mixer/SDL_mixer.h),)
LAZYFOO_APPS := \
	$(filter $(MIXER_INSERT_AFTER),$(LAZYFOO_APPS)) \
	021_sound_effects_music \
	$(filter-out $(MIXER_INSERT_AFTER),$(LAZYFOO_APPS))
endif

.PHONY: all clean release print-apps $(LAZYFOO_APPS)

all: $(LAZYFOO_APPS)

$(LAZYFOO_APPS):
	$(MAKE) -C $@ NXDK_DIR="$(NXDK_DIR)" NXGL_DIR="$(NXGL_DIR)"

clean:
	@for app in $(LAZYFOO_APPS); do $(MAKE) -C $$app clean || exit $$?; done

release: all
	./tools/collect_release_isos.sh

print-apps:
	@printf '%s\n' $(LAZYFOO_APPS)
