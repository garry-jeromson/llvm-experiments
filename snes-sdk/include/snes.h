/*
 * SNES SDK - Unified C API
 *
 * This header provides a complete C API for SNES development.
 * All functions are prefixed with snes_ for namespace safety.
 *
 * For C++ code, prefer the namespaced headers in snes/ directory.
 */

#ifndef SNES_H
#define SNES_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================================================================
 * Type Definitions
 * ============================================================================ */

typedef unsigned char      snes_u8;
typedef unsigned short     snes_u16;
typedef unsigned long      snes_u32;
typedef signed char        snes_i8;
typedef signed short       snes_i16;
typedef signed long        snes_i32;

/* Boolean type for C89 compatibility */
#ifndef __cplusplus
typedef snes_u8 snes_bool;
#define SNES_TRUE  1
#define SNES_FALSE 0
#else
typedef bool snes_bool;
#define SNES_TRUE  true
#define SNES_FALSE false
#endif

/* BGR555 color (native SNES format) */
typedef snes_u16 snes_color;

/* 8.8 fixed-point number */
typedef snes_i16 snes_fixed8;

/* ============================================================================
 * Color Helpers
 * ============================================================================ */

/* Create BGR555 color from RGB components (0-31 each) */
#define SNES_RGB(r, g, b) \
    ((snes_color)(((r) & 0x1F) | (((g) & 0x1F) << 5) | (((b) & 0x1F) << 10)))

/* Extract RGB components from BGR555 color */
#define SNES_RED(c)   ((snes_u8)((c) & 0x1F))
#define SNES_GREEN(c) ((snes_u8)(((c) >> 5) & 0x1F))
#define SNES_BLUE(c)  ((snes_u8)(((c) >> 10) & 0x1F))

/* Common colors */
#define SNES_COLOR_BLACK   SNES_RGB(0, 0, 0)
#define SNES_COLOR_WHITE   SNES_RGB(31, 31, 31)
#define SNES_COLOR_RED     SNES_RGB(31, 0, 0)
#define SNES_COLOR_GREEN   SNES_RGB(0, 31, 0)
#define SNES_COLOR_BLUE    SNES_RGB(0, 0, 31)
#define SNES_COLOR_YELLOW  SNES_RGB(31, 31, 0)
#define SNES_COLOR_CYAN    SNES_RGB(0, 31, 31)
#define SNES_COLOR_MAGENTA SNES_RGB(31, 0, 31)

/* ============================================================================
 * Button Definitions
 * ============================================================================ */

#define SNES_BTN_B      0x8000
#define SNES_BTN_Y      0x4000
#define SNES_BTN_SELECT 0x2000
#define SNES_BTN_START  0x1000
#define SNES_BTN_UP     0x0800
#define SNES_BTN_DOWN   0x0400
#define SNES_BTN_LEFT   0x0200
#define SNES_BTN_RIGHT  0x0100
#define SNES_BTN_A      0x0080
#define SNES_BTN_X      0x0040
#define SNES_BTN_L      0x0020
#define SNES_BTN_R      0x0010

/* ============================================================================
 * Screen Constants
 * ============================================================================ */

#define SNES_SCREEN_WIDTH  256
#define SNES_SCREEN_HEIGHT 224
#define SNES_SCREEN_COLS   32   /* Tiles per row */
#define SNES_SCREEN_ROWS   28   /* Tiles per column */

/* BG mode constants */
#define SNES_MODE0 0  /* 4 BGs, 2bpp each */
#define SNES_MODE1 1  /* 2 BGs 4bpp + 1 BG 2bpp */
#define SNES_MODE2 2  /* 2 BGs 4bpp + offset-per-tile */
#define SNES_MODE3 3  /* 1 BG 8bpp + 1 BG 4bpp */
#define SNES_MODE7 7  /* Rotation/scaling */

/* Screen layer masks */
#define SNES_LAYER_BG1 0x01
#define SNES_LAYER_BG2 0x02
#define SNES_LAYER_BG3 0x04
#define SNES_LAYER_BG4 0x08
#define SNES_LAYER_OBJ 0x10

/* ============================================================================
 * PPU Functions
 * ============================================================================ */

/* Screen control */
void snes_screen_on(snes_u8 brightness);  /* brightness: 0-15 */
void snes_screen_off(void);
void snes_wait_vblank(void);

/* Background color (palette entry 0) */
void snes_set_bgcolor(snes_color color);
void snes_set_bgcolor_rgb(snes_u8 r, snes_u8 g, snes_u8 b);

/* BG mode */
void snes_set_mode(snes_u8 mode);

/* Main/sub screen layers */
void snes_set_main_screen(snes_u8 layer_mask);
void snes_set_sub_screen(snes_u8 layer_mask);

/* ============================================================================
 * Background Functions
 * ============================================================================ */

/* Set BG tilemap address and size */
void snes_bg_set_tilemap(snes_u8 bg, snes_u16 vram_addr, snes_u8 size);

/* Set BG tile data address */
void snes_bg_set_tiles(snes_u8 bg, snes_u16 vram_addr);

/* Set BG scroll position */
void snes_bg_set_scroll(snes_u8 bg, snes_i16 x, snes_i16 y);

/* Enable/disable BG on main screen */
void snes_bg_enable(snes_u8 bg, snes_bool enable);

/* ============================================================================
 * Sprite Functions
 * ============================================================================ */

/* Set sprite position (9-bit x, 8-bit y) */
void snes_sprite_set_pos(snes_u8 id, snes_i16 x, snes_u8 y);

/* Set sprite tile and attributes */
void snes_sprite_set_tile(snes_u8 id, snes_u16 tile, snes_u8 palette,
                          snes_bool hflip, snes_bool vflip);

/* Set sprite priority (0-3) */
void snes_sprite_set_priority(snes_u8 id, snes_u8 priority);

/* Set sprite size (0=small, 1=large) */
void snes_sprite_set_size(snes_u8 id, snes_bool large);

/* Hide a sprite */
void snes_sprite_hide(snes_u8 id);

/* Clear all sprites (hide them) */
void snes_sprites_clear(void);

/* Upload sprite shadow buffer to OAM (call during vblank) */
void snes_sprites_upload(void);

/* Set sprite base address and size mode */
void snes_sprites_set_obsel(snes_u16 base_addr, snes_u8 size_mode);

/* Load built-in sprite tiles to VRAM address 0 */
void snes_load_sprite_tiles(void);

/* Set up sprite palette 0 with yellow smiley colors */
void snes_set_sprite_palette(void);

/* ============================================================================
 * DMA Functions
 * ============================================================================ */

/* Transfer data to VRAM using DMA */
void snes_dma_vram(snes_u8 channel, const void* src, snes_u16 dest, snes_u16 size);

/* Transfer data to CGRAM (palette) using DMA */
void snes_dma_cgram(snes_u8 channel, const void* src, snes_u8 start_color, snes_u16 count);

/* Transfer data to OAM using DMA */
void snes_dma_oam(snes_u8 channel, const void* src, snes_u16 size);

/* ============================================================================
 * Input Functions
 * ============================================================================ */

/* Enable joypad auto-read */
void snes_input_enable(void);

/* Wait for joypad auto-read to complete */
void snes_input_wait(void);

/* Read raw joypad state (0 or 1) */
snes_u16 snes_input_read(snes_u8 joypad_id);

/* Check if button(s) are currently held */
snes_bool snes_joy_held(snes_u8 joypad_id, snes_u16 button_mask);

/* Check if button was just pressed this frame */
snes_bool snes_joy_pressed(snes_u8 joypad_id, snes_u16 button_mask);

/* Check if button was just released this frame */
snes_bool snes_joy_released(snes_u8 joypad_id, snes_u16 button_mask);

/* Update joypad state (call once per frame) */
void snes_joy_update(void);

/* Get D-pad as axis values (-1, 0, 1) */
snes_i8 snes_joy_axis_x(snes_u8 joypad_id);
snes_i8 snes_joy_axis_y(snes_u8 joypad_id);

/* ============================================================================
 * Text Functions
 * ============================================================================ */

/* Initialize text system */
void snes_text_init(snes_u16 tilemap_addr, snes_u16 font_tile_base, snes_u8 palette);

/* Set cursor position */
void snes_text_set_cursor(snes_u8 x, snes_u8 y);

/* Get cursor position */
void snes_text_get_cursor(snes_u8* x, snes_u8* y);

/* Print a character at cursor position */
void snes_text_putchar(char c);

/* Print a string at cursor position */
void snes_text_puts(const char* str);

/* Print a string with newline */
void snes_text_println(const char* str);

/* Clear the text screen */
void snes_text_clear(void);

/* Print numbers */
void snes_text_print_u16(snes_u16 value);
void snes_text_print_i16(snes_i16 value);
void snes_text_print_hex(snes_u16 value);

/* ============================================================================
 * Audio Functions
 * ============================================================================ */

/* Initialize audio system */
snes_bool snes_audio_init(void);

/* Check if audio system is ready */
snes_bool snes_audio_ready(void);

/* Play a sound effect (0-255) */
void snes_audio_play_sfx(snes_u8 sfx_id);

/* Play a music track (0-255) */
void snes_audio_play_music(snes_u8 track_id);

/* Stop music playback */
void snes_audio_stop_music(void);

/* Stop all audio */
void snes_audio_stop_all(void);

/* Set volume levels (0-127) */
void snes_audio_set_master_volume(snes_u8 volume);
void snes_audio_set_sfx_volume(snes_u8 volume);
void snes_audio_set_music_volume(snes_u8 volume);

/* ============================================================================
 * Math Functions
 * ============================================================================ */

/* Sine/cosine lookup (angle: 0-255 = 0-360 degrees, returns 8.8 fixed) */
snes_fixed8 snes_sin(snes_u8 angle);
snes_fixed8 snes_cos(snes_u8 angle);

/* Min/max/clamp */
snes_i16 snes_min(snes_i16 a, snes_i16 b);
snes_i16 snes_max(snes_i16 a, snes_i16 b);
snes_i16 snes_clamp(snes_i16 val, snes_i16 lo, snes_i16 hi);

/* Abs/sign */
snes_i16 snes_abs(snes_i16 val);
snes_i16 snes_sign(snes_i16 val);

/* Linear interpolation (t: 0-255) */
snes_i16 snes_lerp(snes_i16 a, snes_i16 b, snes_u8 t);

/* Distance squared (avoids sqrt) */
snes_i32 snes_dist_sq(snes_i16 x1, snes_i16 y1, snes_i16 x2, snes_i16 y2);

/* Random number generator */
void snes_random_seed(snes_u16 seed);
snes_u16 snes_random(void);
snes_u16 snes_random_range(snes_u16 max);
snes_u16 snes_random_range_minmax(snes_u16 min, snes_u16 max);

/* ============================================================================
 * System Functions
 * ============================================================================ */

/* Initialize SNES hardware (call once at startup) */
void snes_init(void);

/* Wait for N frames */
void snes_wait_frames(snes_u16 count);

#ifdef __cplusplus
}
#endif

#endif /* SNES_H */
