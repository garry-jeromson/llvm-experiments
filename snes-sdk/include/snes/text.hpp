#pragma once

#include "types.hpp"
#include "ppu.hpp"

namespace snes {
namespace text {

// Text cursor position
struct Cursor {
    u8 x;  // Column (0-31)
    u8 y;  // Row (0-27)
};

// Text configuration
struct TextConfig {
    u16 tilemap_addr;   // VRAM word address of tilemap (1KB aligned)
    u16 font_tile_base; // First tile number for font (ASCII 32-127)
    u8 palette;         // Palette number (0-7)
};

// Global text state
extern Cursor g_cursor;
extern TextConfig g_config;

// Initialize text system
// tilemap_addr: VRAM word address for text tilemap
// font_tile_base: First tile number where font starts (maps to ASCII 32)
inline void init(u16 tilemap_addr, u16 font_tile_base, u8 palette = 0) {
    g_config.tilemap_addr = tilemap_addr;
    g_config.font_tile_base = font_tile_base;
    g_config.palette = palette;
    g_cursor.x = 0;
    g_cursor.y = 0;
}

// Set cursor position
inline void set_cursor(u8 x, u8 y) {
    g_cursor.x = x;
    g_cursor.y = y;
}

// Get current cursor position
inline Cursor get_cursor() {
    return g_cursor;
}

// Write a single character at cursor position
// Advances cursor automatically
void putchar(char c);

// Write a null-terminated string at cursor position
void puts(const char* str);

// Write a string with newline
void println(const char* str);

// Clear the text screen (fill with spaces)
void clear();

// Print an unsigned 16-bit integer
void print_u16(u16 value);

// Print a signed 16-bit integer
void print_i16(i16 value);

// Print a hex value (4 digits)
void print_hex(u16 value);

} // namespace text
} // namespace snes
