// SNES Text Display Driver
// Provides simple text output to a background layer

#include <snes/text.hpp>
#include <snes/registers.hpp>

namespace snes {
namespace text {

// Global state
Cursor g_cursor = {0, 0};
TextConfig g_config = {0x1000, 0, 0};

// Write a single character at cursor position
void putchar(char c) {
    // Handle newline
    if (c == '\n') {
        g_cursor.x = 0;
        g_cursor.y++;
        if (g_cursor.y >= 28) g_cursor.y = 0;
        return;
    }

    // Handle carriage return
    if (c == '\r') {
        g_cursor.x = 0;
        return;
    }

    // Handle tab (4 spaces)
    if (c == '\t') {
        g_cursor.x = (g_cursor.x + 4) & 0xFC;
        if (g_cursor.x >= 32) {
            g_cursor.x = 0;
            g_cursor.y++;
            if (g_cursor.y >= 28) g_cursor.y = 0;
        }
        return;
    }

    // Only printable ASCII (32-126)
    if (c < 32 || c > 126) c = '?';

    // Calculate tilemap address
    // Each row is 32 tiles, each tile entry is 2 bytes (tile + attributes)
    u16 offset = (static_cast<u16>(g_cursor.y) * 32 + g_cursor.x);
    u16 vram_addr = g_config.tilemap_addr + offset;

    // Calculate tile number: font_tile_base + (character - 32)
    u16 tile = g_config.font_tile_base + (c - 32);

    // Write to VRAM
    // Set VRAM address (word address)
    reg::VMAIN::write(0x80);  // Increment after high byte write
    reg::VMADDL::write(vram_addr & 0xFF);
    reg::VMADDH::write(vram_addr >> 8);

    // Write tile entry (low byte = tile number, high byte = attributes)
    u8 attr = (g_config.palette << 2);  // Palette bits 2-4
    reg::VMDATAL::write(tile & 0xFF);
    reg::VMDATAH::write(attr | ((tile >> 8) & 0x03));

    // Advance cursor
    g_cursor.x++;
    if (g_cursor.x >= 32) {
        g_cursor.x = 0;
        g_cursor.y++;
        if (g_cursor.y >= 28) g_cursor.y = 0;
    }
}

// Write a null-terminated string
void puts(const char* str) {
    while (*str) {
        putchar(*str++);
    }
}

// Write a string with newline
void println(const char* str) {
    puts(str);
    putchar('\n');
}

// Clear the text screen
void clear() {
    // Set VRAM address to tilemap start
    reg::VMAIN::write(0x80);
    reg::VMADDL::write(g_config.tilemap_addr & 0xFF);
    reg::VMADDH::write(g_config.tilemap_addr >> 8);

    // Space character tile
    u16 space_tile = g_config.font_tile_base;  // ASCII 32 = space
    u8 attr = (g_config.palette << 2);

    // Fill 32x28 = 896 tiles with space
    for (u16 i = 0; i < 32 * 28; i++) {
        reg::VMDATAL::write(space_tile & 0xFF);
        reg::VMDATAH::write(attr | ((space_tile >> 8) & 0x03));
    }

    // Reset cursor
    g_cursor.x = 0;
    g_cursor.y = 0;
}

// Print unsigned 16-bit integer
void print_u16(u16 value) {
    char buf[6];  // Max 65535 = 5 digits + null
    int i = 5;
    buf[i] = '\0';

    if (value == 0) {
        putchar('0');
        return;
    }

    while (value > 0 && i > 0) {
        i--;
        buf[i] = '0' + (value % 10);
        value /= 10;
    }

    puts(&buf[i]);
}

// Print signed 16-bit integer
void print_i16(i16 value) {
    if (value < 0) {
        putchar('-');
        value = -value;
    }
    print_u16(static_cast<u16>(value));
}

// Print hex value (4 digits)
void print_hex(u16 value) {
    static const char hex[] = "0123456789ABCDEF";
    putchar(hex[(value >> 12) & 0xF]);
    putchar(hex[(value >> 8) & 0xF]);
    putchar(hex[(value >> 4) & 0xF]);
    putchar(hex[value & 0xF]);
}

} // namespace text
} // namespace snes
