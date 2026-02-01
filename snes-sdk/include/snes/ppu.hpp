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

// Set screen brightness (0-15) and enable display
// brightness: 0=black, 15=full brightness
inline void screen_on(u8 brightness) {
    hal::write8(reg::INIDISP::address, brightness & 0x0F);
}

// Force blank (screen off)
inline void screen_off() {
    hal::write8(reg::INIDISP::address, 0x80);
}

// Wait for vertical blank period
inline void wait_vblank() {
    while ((hal::read8(reg::HVBJOY::address) & 0x80) == 0) {}
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

} // namespace snes::ppu
