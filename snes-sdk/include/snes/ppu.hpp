#pragma once

#include "types.hpp"
#include "hal.hpp"
#include "registers.hpp"

namespace snes::ppu {

// Screen control

// Turn on the screen with specified brightness (0-15)
void screen_on(u8 brightness = 15);

// Turn off the screen (force blank)
void screen_off();

// Wait for vertical blank period
void wait_vblank();

// Check if currently in vblank
bool in_vblank();

// Set background color (color index 0)
void set_bgcolor(u8 r, u8 g, u8 b);
void set_bgcolor(Color color);

// Set BG mode (0-7)
void set_mode(u8 mode);

// Background layer class
class Background {
    u8 m_id;  // 0-3 for BG1-BG4

public:
    explicit Background(u8 id) : m_id(id) {}

    // Set tilemap address in VRAM (word address, typically $0000-$7C00)
    // size: 0 = 32x32, 1 = 64x32, 2 = 32x64, 3 = 64x64
    void set_tilemap(u16 vram_addr, u8 size = 0);

    // Set character data address in VRAM (word address)
    // bpp8: false = 4bpp (16 colors), true = 8bpp (256 colors)
    void set_tiles(u16 vram_addr, bool bpp8 = false);

    // Set scroll position
    void scroll(i16 x, i16 y);

    // Enable this background on main screen
    void enable();

    // Disable this background on main screen
    void disable();

    // Enable/disable on sub screen (for color math)
    void enable_sub();
    void disable_sub();

    // Enable 16x16 tile mode for this BG
    void set_tile_size(bool large);

    // Get BG ID (0-3)
    u8 id() const { return m_id; }
};

// Sprite management

// OAM entry (4 bytes in low table)
struct OAMEntry {
    u8 x_low;      // X position (low 8 bits)
    u8 y;          // Y position
    u8 tile;       // Tile number (low 8 bits)
    u8 attr;       // Attributes: vhoopppc (flip, priority, palette, tile high bit)
};

// OAM high table entry (1 bit X high, 1 bit size, packed 4 sprites per byte)
// Bit layout: [s3 x3 s2 x2 s1 x1 s0 x0]

// Sprite class - manages a single sprite
class Sprite {
    u8 m_id;  // 0-127

public:
    explicit Sprite(u8 id) : m_id(id) {}

    // Set sprite position (x: -256 to 255, y: 0-255, wraps)
    void set_pos(i16 x, i16 y);

    // Set tile and attributes
    // tile: 0-511 (9-bit tile number)
    // palette: 0-7 (sprite palette)
    // hflip/vflip: horizontal/vertical flip
    void set_tile(u16 tile, u8 palette = 0, bool hflip = false, bool vflip = false);

    // Set priority (0-3, higher = in front)
    void set_priority(u8 prio);

    // Set size (false = small, true = large, based on OBSEL)
    void set_size(bool large);

    // Hide sprite (move off-screen)
    void hide();

    // Get sprite ID (0-127)
    u8 id() const { return m_id; }
};

// Shadow OAM buffer
extern OAMEntry oam_low[128];   // 512 bytes
extern u8 oam_high[32];         // 32 bytes (high bits for 128 sprites)

// Upload shadow OAM to PPU (call during vblank)
void sprites_update();

// Clear all sprites (hide them)
void sprites_clear();

// Enable/disable sprites on main screen
void sprites_enable();
void sprites_disable();

// Set sprite tile base address and size
// base: VRAM word address for sprite tiles
// size: 0-7 (see OBSEL documentation)
//   0: 8x8, 16x16    1: 8x8, 32x32    2: 8x8, 64x64
//   3: 16x16, 32x32  4: 16x16, 64x64  5: 32x32, 64x64
void sprites_set_base(u16 base, u8 size_select = 0);

// Mode 7 (rotation/scaling)
namespace mode7 {
    // Initialize Mode 7
    void init();

    // Set transformation matrix (8.8 fixed point values)
    // [ a  b ]   applied to (x - cx, y - cy) + (hscroll, vscroll)
    // [ c  d ]
    void set_matrix(i16 a, i16 b, i16 c, i16 d);

    // Set rotation center point
    void set_center(i16 x, i16 y);

    // Set scroll offset
    void set_scroll(i16 x, i16 y);

    // Helper: set rotation (angle in 0-255 units, 256 = full rotation)
    void set_rotation(u8 angle, Fixed8 scale = Fixed8::from_int(1));

    // Set mode 7 flags
    // flip_x, flip_y: flip the plane
    // wrap: 0 = transparent outside, 1 = wrap/tile
    void set_flags(bool flip_x = false, bool flip_y = false, bool wrap = true);
}

// Text output (uses 8x8 ASCII font)
// These write directly to VRAM tilemap

// Put a single character at tile position
void put_char(u16 x, u16 y, char c);

// Put a null-terminated string starting at tile position
void put_text(u16 x, u16 y, const char* str);

// Put a number (decimal) at tile position
void put_number(u16 x, u16 y, u16 num);

// Put a hex number at tile position
void put_hex(u16 x, u16 y, u16 num);

// Clear the screen (fill tilemap with space character)
void clear_text();

// Set text color (palette index for text)
void set_text_palette(u8 palette);

// Set tilemap address for text output
void set_text_tilemap(u16 vram_addr);

// Upload default font to VRAM
void upload_font(u16 vram_addr);

} // namespace snes::ppu
