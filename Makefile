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
