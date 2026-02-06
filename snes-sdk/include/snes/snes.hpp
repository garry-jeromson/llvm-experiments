#pragma once

// SNES SDK - C++ Umbrella Header
// Include this single header to get access to all SDK functionality

#include "types.hpp"
#include "hal.hpp"
#include "registers.hpp"
#include "ppu.hpp"
#include "input.hpp"
#include "audio.hpp"
#include "text.hpp"
#include "math.hpp"
#include "dma.hpp"

namespace snes {

// ============================================================================
// System Initialization
// ============================================================================

// Initialize the SNES hardware (call once at startup)
inline void init() {
    // Force blank during initialization
    ppu::screen_off();

    // Clear sprites
    ppu::sprites_clear();

    // Enable joypad auto-read
    input::enable_joypad();

    // Set default mode 1
    ppu::set_mode(1);

    // Set black background
    ppu::set_bgcolor(Color(0));
}

// ============================================================================
// Background Class (convenience wrapper)
// ============================================================================

namespace ppu {

class Background {
    u8 m_id;  // Background number (1-4)

public:
    explicit Background(u8 id) : m_id(id) {}

    // Set tilemap VRAM address and size
    // addr: word address (must be 1KB aligned)
    // size: 0=32x32, 1=64x32, 2=32x64, 3=64x64
    void set_tilemap(u16 addr, u8 size = 0) {
        u8 val = make_bgsc(addr, size);
        switch (m_id) {
            case 1: set_bg1sc(val); break;
            case 2: set_bg2sc(val); break;
            case 3: set_bg3sc(val); break;
            case 4: set_bg4sc(val); break;
        }
    }

    // Set tile data VRAM address
    // addr: word address (must be 4KB aligned for 2bpp, 8KB for 4bpp)
    void set_tiles(u16 addr) {
        u8 nibble = static_cast<u8>((addr >> 12) & 0x0F);
        switch (m_id) {
            case 1: {
                u8 current = hal::read8(reg::BG12NBA::address);
                set_bg12nba((current & 0xF0) | nibble);
                break;
            }
            case 2: {
                u8 current = hal::read8(reg::BG12NBA::address);
                set_bg12nba((current & 0x0F) | (nibble << 4));
                break;
            }
            case 3: {
                u8 current = hal::read8(reg::BG34NBA::address);
                set_bg34nba((current & 0xF0) | nibble);
                break;
            }
            case 4: {
                u8 current = hal::read8(reg::BG34NBA::address);
                set_bg34nba((current & 0x0F) | (nibble << 4));
                break;
            }
        }
    }

    // Set scroll position
    void set_scroll(i16 x, i16 y) {
        switch (m_id) {
            case 1:
                set_bg1hofs_lo(static_cast<u8>(x & 0xFF));
                set_bg1hofs_hi(static_cast<u8>(x >> 8));
                set_bg1vofs_lo(static_cast<u8>(y & 0xFF));
                set_bg1vofs_hi(static_cast<u8>(y >> 8));
                break;
            case 2:
                set_bg2hofs_lo(static_cast<u8>(x & 0xFF));
                set_bg2hofs_hi(static_cast<u8>(x >> 8));
                set_bg2vofs_lo(static_cast<u8>(y & 0xFF));
                set_bg2vofs_hi(static_cast<u8>(y >> 8));
                break;
            case 3:
                set_bg3hofs_lo(static_cast<u8>(x & 0xFF));
                set_bg3hofs_hi(static_cast<u8>(x >> 8));
                set_bg3vofs_lo(static_cast<u8>(y & 0xFF));
                set_bg3vofs_hi(static_cast<u8>(y >> 8));
                break;
        }
    }

    // Enable this background on main screen
    void enable() {
        u8 mask = static_cast<u8>(1 << (m_id - 1));
        u8 current = hal::read8(reg::TM::address);
        set_tm(current | mask);
    }

    // Disable this background on main screen
    void disable() {
        u8 mask = static_cast<u8>(1 << (m_id - 1));
        u8 current = hal::read8(reg::TM::address);
        set_tm(current & ~mask);
    }
};

// Convenience overload: set_bgcolor with RGB values
inline void set_bgcolor(u8 r, u8 g, u8 b) {
    set_bgcolor(Color::from_rgb(r, g, b));
}

// Convenience: screen_on with default brightness
inline void screen_on() {
    screen_on(15);  // Full brightness
}

// Convenience: upload sprites (alias)
inline void sprites_update() {
    sprites_upload();
}

} // namespace ppu

} // namespace snes
