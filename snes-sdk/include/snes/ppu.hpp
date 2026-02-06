#pragma once

// SNES PPU API - Header-only PPU functions for W65816 backend
// All functions are inline single register writes to avoid register pressure

#include "types.hpp"
#include "hal.hpp"
#include "registers.hpp"

namespace snes::ppu {

// OAM entry (4 bytes in low table)
struct OAMEntry {
    u8 x_low;      // X position (low 8 bits)
    u8 y;          // Y position
    u8 tile;       // Tile number (low 8 bits)
    u8 attr;       // Attributes: vhoopppc (flip, priority, palette, tile high bit)
};

// ============================================================================
// Screen Control
// ============================================================================

// Brightness value mask - bits 0-3 of INIDISP control brightness (0-15)
static constexpr u8 BRIGHTNESS_MASK = 0x0F;

// Set screen brightness (0-15) and enable display
// brightness: 0=black, 15=full brightness
inline void screen_on(u8 brightness) {
    hal::write8(reg::INIDISP::address, brightness & BRIGHTNESS_MASK);
}

// Force blank (screen off)
inline void screen_off() {
    hal::write8(reg::INIDISP::address, 0x80);
}

// Wait for next vertical blank period
// First wait for vblank to end (if in vblank), then wait for it to start
inline void wait_vblank() {
    volatile u8* hvbjoy = reinterpret_cast<volatile u8*>(0x4212);
    // Wait for vblank to end (bit 7 clear)
    while (*hvbjoy & 0x80) {}
    // Wait for vblank to start (bit 7 set)
    while ((*hvbjoy & 0x80) == 0) {}
}

// ============================================================================
// Background Color (Palette Entry 0)
// ============================================================================

// Set background color using Color type (BGR555 format)
inline void set_bgcolor(Color c) {
    hal::write8(reg::CGADD::address, 0);
    hal::write8(reg::CGDATA::address, c.raw & 0xFF);
    hal::write8(reg::CGDATA::address, c.raw >> 8);
}

// Set background color using raw lo/hi bytes (legacy)
inline void set_bgcolor_lo(u8 lo) {
    hal::write8(reg::CGADD::address, 0);
    hal::write8(reg::CGDATA::address, lo);
}
inline void set_bgcolor_hi(u8 hi) {
    hal::write8(reg::CGDATA::address, hi);
}

// ============================================================================
// Background Mode
// ============================================================================

// Set BG mode (0-7)
// Mode 0: 4 BGs, 2bpp each (4 colors per BG)
// Mode 1: 2 BGs 4bpp + 1 BG 2bpp (most common)
// Mode 2: 2 BGs 4bpp + offset-per-tile
// Mode 3: 1 BG 8bpp + 1 BG 4bpp
// Mode 7: Rotation/scaling
inline void set_mode(u8 mode) {
    hal::write8(reg::BGMODE::address, mode);
}

// ============================================================================
// Background Tilemap Address
// ============================================================================

// Set BG tilemap address and size
// val: (vram_addr >> 8) | size_bits
// Use make_bgsc() helper to compute
inline void set_bg1sc(u8 val) { hal::write8(reg::BG1SC::address, val); }
inline void set_bg2sc(u8 val) { hal::write8(reg::BG2SC::address, val); }
inline void set_bg3sc(u8 val) { hal::write8(reg::BG3SC::address, val); }
inline void set_bg4sc(u8 val) { hal::write8(reg::BG4SC::address, val); }

// ============================================================================
// Background Tile Data Address
// ============================================================================

// Set BG1/BG2 character (tile) data base address
// Low nibble = BG1 base >> 12, High nibble = BG2 base >> 12
inline void set_bg12nba(u8 val) { hal::write8(reg::BG12NBA::address, val); }

// Set BG3/BG4 character (tile) data base address
inline void set_bg34nba(u8 val) { hal::write8(reg::BG34NBA::address, val); }

// ============================================================================
// Background Scroll
// ============================================================================

// BG1 scroll (write twice: low byte, then high byte)
inline void set_bg1hofs_lo(u8 lo) { hal::write8(reg::BG1HOFS::address, lo); }
inline void set_bg1hofs_hi(u8 hi) { hal::write8(reg::BG1HOFS::address, hi); }
inline void set_bg1vofs_lo(u8 lo) { hal::write8(reg::BG1VOFS::address, lo); }
inline void set_bg1vofs_hi(u8 hi) { hal::write8(reg::BG1VOFS::address, hi); }

// BG2 scroll
inline void set_bg2hofs_lo(u8 lo) { hal::write8(reg::BG2HOFS::address, lo); }
inline void set_bg2hofs_hi(u8 hi) { hal::write8(reg::BG2HOFS::address, hi); }
inline void set_bg2vofs_lo(u8 lo) { hal::write8(reg::BG2VOFS::address, lo); }
inline void set_bg2vofs_hi(u8 hi) { hal::write8(reg::BG2VOFS::address, hi); }

// BG3 scroll
inline void set_bg3hofs_lo(u8 lo) { hal::write8(reg::BG3HOFS::address, lo); }
inline void set_bg3hofs_hi(u8 hi) { hal::write8(reg::BG3HOFS::address, hi); }
inline void set_bg3vofs_lo(u8 lo) { hal::write8(reg::BG3VOFS::address, lo); }
inline void set_bg3vofs_hi(u8 hi) { hal::write8(reg::BG3VOFS::address, hi); }

// ============================================================================
// Main/Sub Screen Designation
// ============================================================================

// Set main screen layers (use SCREEN_* constants)
inline void set_tm(u8 mask) { hal::write8(reg::TM::address, mask); }

// Set sub screen layers (for color math)
inline void set_ts(u8 mask) { hal::write8(reg::TS::address, mask); }

// Screen mask bits
static constexpr u8 SCREEN_BG1 = 0x01;
static constexpr u8 SCREEN_BG2 = 0x02;
static constexpr u8 SCREEN_BG3 = 0x04;
static constexpr u8 SCREEN_BG4 = 0x08;
static constexpr u8 SCREEN_OBJ = 0x10;

// ============================================================================
// Sprite (OBJ) Settings
// ============================================================================

// Set sprite size and VRAM base
// Use make_obsel() helper to compute value
// Size modes: 0=8x8/16x16, 1=8x8/32x32, 2=8x8/64x64, 3=16x16/32x32, etc.
inline void set_obsel(u8 val) { hal::write8(reg::OBSEL::address, val); }

// ============================================================================
// VRAM Access
// ============================================================================

// Set VRAM address increment mode
// 0x80 = increment on high byte write (word access)
// 0x00 = increment on low byte write
inline void set_vmain(u8 val) { hal::write8(reg::VMAIN::address, val); }

// Set VRAM address (16-bit word address)
inline void set_vmaddr(u16 addr) {
    hal::write8(reg::VMADDL::address, addr & 0xFF);
    hal::write8(reg::VMADDH::address, addr >> 8);
}

// Set VRAM address (low/high bytes separately)
inline void set_vmaddl(u8 lo) { hal::write8(reg::VMADDL::address, lo); }
inline void set_vmaddh(u8 hi) { hal::write8(reg::VMADDH::address, hi); }

// Write VRAM data
inline void set_vmdatal(u8 lo) { hal::write8(reg::VMDATAL::address, lo); }
inline void set_vmdatah(u8 hi) { hal::write8(reg::VMDATAH::address, hi); }

// ============================================================================
// CGRAM (Palette) Access
// ============================================================================

// Set CGRAM (palette) address (0-255, 256 colors)
inline void set_cgadd(u8 color) { hal::write8(reg::CGADD::address, color); }

// Write CGRAM data (write twice for 15-bit color: low, high)
inline void set_cgdata(u8 val) { hal::write8(reg::CGDATA::address, val); }

// ============================================================================
// OAM (Sprite Table) Access
// ============================================================================

// Set OAM address (byte address into 544-byte OAM)
inline void set_oamaddl(u8 lo) { hal::write8(reg::OAMADDL::address, lo); }
inline void set_oamaddh(u8 hi) { hal::write8(reg::OAMADDH::address, hi); }

// Set OAM address (16-bit combined)
inline void set_oamaddr(u16 addr) {
    hal::write8(reg::OAMADDL::address, addr & 0xFF);
    hal::write8(reg::OAMADDH::address, addr >> 8);
}

// Write OAM data
inline void write_oamdata(u8 val) { hal::write8(reg::OAMDATA::address, val); }

// ============================================================================
// DMA Channel 0 (General Purpose DMA)
// ============================================================================

// DMA control register
// Bits 0-2: Transfer mode
// Bit 3: Address step (0=inc, 1=dec)
// Bit 4: Reserved
// Bit 6: HDMA indirect
// Bit 7: Direction (0=A→B read, 1=B→A write)
inline void set_dmap0(u8 val) { hal::write8(reg::DMA<0>::CTRL::address, val); }

// DMA destination (B-bus address, $21xx low byte)
inline void set_bbad0(u8 val) { hal::write8(reg::DMA<0>::DEST::address, val); }

// DMA source address
inline void set_a1t0l(u8 lo) { hal::write8(reg::DMA<0>::SRCL::address, lo); }
inline void set_a1t0h(u8 hi) { hal::write8(reg::DMA<0>::SRCM::address, hi); }
inline void set_a1b0(u8 bank) { hal::write8(reg::DMA<0>::SRCH::address, bank); }

// DMA transfer size
inline void set_das0l(u8 lo) { hal::write8(reg::DMA<0>::SIZEL::address, lo); }
inline void set_das0h(u8 hi) { hal::write8(reg::DMA<0>::SIZEH::address, hi); }

// Start DMA transfer (bit mask for channels 0-7)
inline void start_dma(u8 mask) { hal::write8(reg::MDMAEN::address, mask); }

// ============================================================================
// Mosaic Effect
// ============================================================================

// Set mosaic size and enable for backgrounds
// size: 0-15 (0=1x1, 1=2x2, ..., 15=16x16)
// bg_mask: bit 0=BG1, bit 1=BG2, bit 2=BG3, bit 3=BG4
inline void set_mosaic(u8 size, u8 bg_mask) {
    hal::write8(reg::MOSAIC::address, ((size & 0x0F) << 4) | (bg_mask & 0x0F));
}

// ============================================================================
// Mode 7 Settings
// ============================================================================

// Set Mode 7 selection register
// Bit 0: Screen flip H
// Bit 1: Screen flip V
// Bits 6-7: Screen over (0=wrap, 1=wrap, 2=transparent, 3=tile 0)
inline void set_m7sel(u8 val) { hal::write8(reg::M7SEL::address, val); }

// Mode 7 matrix parameters (all write twice: low then high byte)
// M7A = cos(angle) * scale_x
// M7B = sin(angle) * scale_x (also multiplicand for hardware multiply)
// M7C = -sin(angle) * scale_y
// M7D = cos(angle) * scale_y
inline void set_m7a(i16 val) {
    hal::write8(reg::M7A::address, val & 0xFF);
    hal::write8(reg::M7A::address, val >> 8);
}

inline void set_m7b(i16 val) {
    hal::write8(reg::M7B::address, val & 0xFF);
    hal::write8(reg::M7B::address, val >> 8);
}

inline void set_m7c(i16 val) {
    hal::write8(reg::M7C::address, val & 0xFF);
    hal::write8(reg::M7C::address, val >> 8);
}

inline void set_m7d(i16 val) {
    hal::write8(reg::M7D::address, val & 0xFF);
    hal::write8(reg::M7D::address, val >> 8);
}

// Mode 7 center point (origin for rotation/scaling)
inline void set_m7x(i16 val) {
    hal::write8(reg::M7X::address, val & 0xFF);
    hal::write8(reg::M7X::address, val >> 8);
}

inline void set_m7y(i16 val) {
    hal::write8(reg::M7Y::address, val & 0xFF);
    hal::write8(reg::M7Y::address, val >> 8);
}

// ============================================================================
// HDMA Control
// ============================================================================

// Enable HDMA channels (bit mask for channels 0-7)
inline void enable_hdma(u8 channels) { hal::write8(reg::HDMAEN::address, channels); }

// Disable all HDMA
inline void disable_hdma() { hal::write8(reg::HDMAEN::address, 0); }

// ============================================================================
// Color Math
// ============================================================================

// Set color math selection
// Bits 0-1: Direct color / screen add/sub select
// Bits 4-5: Main screen black enable
// Bits 6-7: Color math enable
inline void set_cgwsel(u8 val) { hal::write8(reg::CGWSEL::address, val); }

// Set color math designation
// Bits 0-5: Enable color math for BG1-4, OBJ, backdrop
// Bit 6: Half color math
// Bit 7: Add/subtract mode (0=add, 1=sub)
inline void set_cgadsub(u8 val) { hal::write8(reg::CGADSUB::address, val); }

// Set fixed color for color math
// Bits 0-4: Color intensity (0-31)
// Bit 5: Blue channel
// Bit 6: Green channel
// Bit 7: Red channel
inline void set_coldata(u8 val) { hal::write8(reg::COLDATA::address, val); }

// Set fixed color RGB values
inline void set_fixed_color(u8 r, u8 g, u8 b) {
    hal::write8(reg::COLDATA::address, 0x20 | (b & 0x1F));  // Blue
    hal::write8(reg::COLDATA::address, 0x40 | (g & 0x1F));  // Green
    hal::write8(reg::COLDATA::address, 0x80 | (r & 0x1F));  // Red
}

// ============================================================================
// Helper Functions
// ============================================================================

// Compute BGnSC register value
// vram_addr: word address of tilemap in VRAM (must be 1KB aligned)
// size: 0=32x32, 1=64x32, 2=32x64, 3=64x64
inline u8 make_bgsc(u16 vram_addr, u8 size) {
    return static_cast<u8>(((vram_addr >> 8) & 0xFC) | (size & 0x03));
}

// Compute OBSEL register value
// base: word address of sprite tiles in VRAM (must be 8KB aligned)
// size: sprite size mode (0-7)
inline u8 make_obsel(u16 base, u8 size) {
    return static_cast<u8>(((base >> 13) & 0x07) | ((size & 0x07) << 5));
}

// Mode 7 selection flags
namespace m7sel {
    constexpr u8 FLIP_H    = 0x01;  // Horizontal flip
    constexpr u8 FLIP_V    = 0x02;  // Vertical flip
    constexpr u8 OVER_WRAP = 0x00;  // Wrap on screen edges
    constexpr u8 OVER_TRANSPARENT = 0x80;  // Transparent outside
    constexpr u8 OVER_TILE0 = 0xC0;  // Use tile 0 outside
}

// Color math flags
namespace cgadsub {
    constexpr u8 BG1      = 0x01;
    constexpr u8 BG2      = 0x02;
    constexpr u8 BG3      = 0x04;
    constexpr u8 BG4      = 0x08;
    constexpr u8 OBJ      = 0x10;
    constexpr u8 BACKDROP = 0x20;
    constexpr u8 HALF     = 0x40;
    constexpr u8 SUBTRACT = 0x80;
}

// ============================================================================
// OAM Shadow Buffers (for sprite management)
// ============================================================================

// Low table: 128 sprites × 4 bytes = 512 bytes
// High table: 128 sprites × 2 bits = 32 bytes (packed)
// These are shadow buffers in RAM; call sprites_upload() to copy to OAM

#ifdef SNES_TESTING
// For unit tests, these are defined in the test
extern OAMEntry oam_low[128];
extern u8 oam_high[32];
#else
// For production, these are statically allocated
inline OAMEntry oam_low[128];
inline u8 oam_high[32];
#endif

// ============================================================================
// Sprite Class (OAM entry wrapper)
// ============================================================================

class Sprite {
    u8 m_id;  // Sprite index (0-127)

public:
    explicit Sprite(u8 id) : m_id(id) {}

    // Get sprite ID
    u8 id() const { return m_id; }

    // Set sprite position
    // x: -256 to 511 (9-bit signed, wraps at screen edges)
    // y: 0-255 (on-screen 0-223, 224-255 hidden, 240 = hide)
    void set_pos(i16 x, u8 y) {
        oam_low[m_id].x_low = static_cast<u8>(x & 0xFF);
        oam_low[m_id].y = y;

        // Set X high bit in oam_high
        // Each byte holds 4 sprites: bits 0,2,4,6 = X high bits
        // Sprite n: byte n/4, bit (n%4)*2
        u8 byte_idx = m_id >> 2;
        u8 bit_pos = (m_id & 0x03) << 1;
        u8 x_high = (x & 0x100) ? 1 : 0;

        oam_high[byte_idx] = static_cast<u8>(
            (oam_high[byte_idx] & ~(0x01 << bit_pos)) | (x_high << bit_pos)
        );
    }

    // Set tile number (0-511)
    // palette: 0-7
    // hflip: horizontal flip
    // vflip: vertical flip
    void set_tile(u16 tile, u8 palette = 0, bool hflip = false, bool vflip = false) {
        oam_low[m_id].tile = static_cast<u8>(tile & 0xFF);
        oam_low[m_id].attr = static_cast<u8>(
            ((tile >> 8) & 0x01) |           // Bit 0: tile high bit
            ((palette & 0x07) << 1) |        // Bits 1-3: palette
            (oam_low[m_id].attr & 0x30) |    // Bits 4-5: priority (preserved)
            (hflip ? 0x40 : 0) |             // Bit 6: H-flip
            (vflip ? 0x80 : 0)               // Bit 7: V-flip
        );
    }

    // Set sprite priority (0-3, 0=lowest, 3=highest)
    void set_priority(u8 prio) {
        oam_low[m_id].attr = static_cast<u8>(
            (oam_low[m_id].attr & ~0x30) | ((prio & 0x03) << 4)
        );
    }

    // Set sprite size (false=small, true=large)
    // Actual size depends on OBSEL register setting
    void set_size(bool large) {
        // Size bit is at position 1 in each sprite's high table entry
        // Sprite n: byte n/4, bit (n%4)*2 + 1
        u8 byte_idx = m_id >> 2;
        u8 bit_pos = ((m_id & 0x03) << 1) + 1;

        if (large) {
            oam_high[byte_idx] |= static_cast<u8>(1 << bit_pos);
        } else {
            oam_high[byte_idx] &= static_cast<u8>(~(1 << bit_pos));
        }
    }

    // Hide sprite by moving off-screen
    void hide() {
        oam_low[m_id].y = 240;  // Below visible area
    }
};

// ============================================================================
// Sprite Management Functions
// ============================================================================

// Clear all sprites (hide them all)
inline void sprites_clear() {
    for (int i = 0; i < 128; i++) {
        oam_low[i].x_low = 0;
        oam_low[i].y = 240;  // Off-screen
        oam_low[i].tile = 0;
        oam_low[i].attr = 0;
    }
    for (int i = 0; i < 32; i++) {
        oam_high[i] = 0;
    }
}

// Upload shadow OAM to hardware OAM via DMA
// Should be called during VBlank
inline void sprites_upload() {
    // Set OAM address to 0
    set_oamaddr(0);

    // DMA channel 0: transfer oam_low (512 bytes) + oam_high (32 bytes)
    // Total: 544 bytes

    // Set DMA mode: A→B, 8-bit, auto-increment
    set_dmap0(0x00);

    // Set destination: OAMDATA ($2104)
    set_bbad0(0x04);

    // Set source address (oam_low)
    u32 src = reinterpret_cast<u32>(&oam_low[0]);
    set_a1t0l(static_cast<u8>(src & 0xFF));
    set_a1t0h(static_cast<u8>((src >> 8) & 0xFF));
    set_a1b0(static_cast<u8>((src >> 16) & 0xFF));

    // Set transfer size: 544 bytes (0x220)
    set_das0l(0x20);
    set_das0h(0x02);

    // Start DMA channel 0
    start_dma(0x01);
}

} // namespace snes::ppu
