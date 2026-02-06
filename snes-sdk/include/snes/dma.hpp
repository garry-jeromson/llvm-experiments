#pragma once

// SNES DMA API - Header-only DMA functions for W65816 backend
// Direct Memory Access for high-speed data transfers to PPU

#include "types.hpp"
#include "hal.hpp"
#include "registers.hpp"

namespace snes::dma {

// ============================================================================
// DMA Transfer Modes
// ============================================================================

// Transfer mode (bits 0-2 of DMAPn)
namespace mode {
    constexpr u8 BYTE_TO_ONE     = 0x00;  // 1 byte to 1 register
    constexpr u8 WORD_TO_TWO     = 0x01;  // 2 bytes to 2 registers (lo, hi)
    constexpr u8 BYTE_TO_TWO     = 0x02;  // 1 byte each to 2 registers
    constexpr u8 WORD_TO_TWO_X2  = 0x03;  // 2 bytes each to 2 registers
    constexpr u8 BYTE_TO_FOUR    = 0x04;  // 1 byte each to 4 registers
}

// Address increment mode (bit 3-4 of DMAPn)
namespace addr {
    constexpr u8 INCREMENT = 0x00;  // Increment after each transfer
    constexpr u8 DECREMENT = 0x10;  // Decrement after each transfer
    constexpr u8 FIXED     = 0x08;  // Don't change address
}

// Direction (bit 7 of DMAPn)
namespace dir {
    constexpr u8 TO_PPU   = 0x00;  // A-bus (CPU) → B-bus (PPU)
    constexpr u8 FROM_PPU = 0x80;  // B-bus (PPU) → A-bus (CPU)
}

// ============================================================================
// Low-Level DMA Channel Access
// ============================================================================

// Set DMA channel control register
template<u8 Channel>
inline void set_control(u8 mode_flags) {
    static_assert(Channel < 8, "DMA channel must be 0-7");
    hal::write8(reg::DMA<Channel>::CTRL::address, mode_flags);
}

// Set B-bus destination register ($21xx low byte)
template<u8 Channel>
inline void set_dest(u8 dest) {
    static_assert(Channel < 8, "DMA channel must be 0-7");
    hal::write8(reg::DMA<Channel>::DEST::address, dest);
}

// Set A-bus source address (24-bit)
template<u8 Channel>
inline void set_source(u32 addr) {
    static_assert(Channel < 8, "DMA channel must be 0-7");
    hal::write8(reg::DMA<Channel>::SRCL::address, static_cast<u8>(addr & 0xFF));
    hal::write8(reg::DMA<Channel>::SRCM::address, static_cast<u8>((addr >> 8) & 0xFF));
    hal::write8(reg::DMA<Channel>::SRCH::address, static_cast<u8>((addr >> 16) & 0xFF));
}

// Set transfer size (16-bit)
template<u8 Channel>
inline void set_size(u16 size) {
    static_assert(Channel < 8, "DMA channel must be 0-7");
    hal::write8(reg::DMA<Channel>::SIZEL::address, static_cast<u8>(size & 0xFF));
    hal::write8(reg::DMA<Channel>::SIZEH::address, static_cast<u8>(size >> 8));
}

// Start DMA transfer on specified channels (bitmask)
inline void start(u8 channel_mask) {
    hal::write8(reg::MDMAEN::address, channel_mask);
}

// ============================================================================
// High-Level Transfer Functions
// ============================================================================

// Transfer data to VRAM
// channel: DMA channel to use (0-7)
// src: Source address in CPU memory
// vram_addr: Destination word address in VRAM
// size: Number of bytes to transfer
template<u8 Channel = 0>
inline void transfer_to_vram(const void* src, u16 vram_addr, u16 size) {
    static_assert(Channel < 8, "DMA channel must be 0-7");

    // Set VRAM address
    hal::write8(reg::VMAIN::address, 0x80);  // Increment on high byte write
    hal::write8(reg::VMADDL::address, static_cast<u8>(vram_addr & 0xFF));
    hal::write8(reg::VMADDH::address, static_cast<u8>(vram_addr >> 8));

    // Configure DMA
    set_control<Channel>(mode::WORD_TO_TWO | addr::INCREMENT | dir::TO_PPU);
    set_dest<Channel>(0x18);  // VMDATAL
    set_source<Channel>(reinterpret_cast<u32>(src));
    set_size<Channel>(size);

    // Start transfer
    start(static_cast<u8>(1 << Channel));
}

// Transfer data to CGRAM (palette)
// channel: DMA channel to use (0-7)
// src: Source address (array of BGR555 colors)
// start_color: First palette entry to write (0-255)
// count: Number of colors (in bytes: colors * 2)
template<u8 Channel = 0>
inline void transfer_to_cgram(const void* src, u8 start_color, u16 count) {
    static_assert(Channel < 8, "DMA channel must be 0-7");

    // Set CGRAM address
    hal::write8(reg::CGADD::address, start_color);

    // Configure DMA
    set_control<Channel>(mode::BYTE_TO_ONE | addr::INCREMENT | dir::TO_PPU);
    set_dest<Channel>(0x22);  // CGDATA
    set_source<Channel>(reinterpret_cast<u32>(src));
    set_size<Channel>(count);

    // Start transfer
    start(static_cast<u8>(1 << Channel));
}

// Transfer data to OAM
// channel: DMA channel to use (0-7)
// src: Source address (OAM shadow buffer)
// size: Number of bytes (max 544)
template<u8 Channel = 0>
inline void transfer_to_oam(const void* src, u16 size = 544) {
    static_assert(Channel < 8, "DMA channel must be 0-7");

    // Set OAM address to 0
    hal::write8(reg::OAMADDL::address, 0);
    hal::write8(reg::OAMADDH::address, 0);

    // Configure DMA
    set_control<Channel>(mode::BYTE_TO_ONE | addr::INCREMENT | dir::TO_PPU);
    set_dest<Channel>(0x04);  // OAMDATA
    set_source<Channel>(reinterpret_cast<u32>(src));
    set_size<Channel>(size);

    // Start transfer
    start(static_cast<u8>(1 << Channel));
}

// ============================================================================
// HDMA (Horizontal Blank DMA)
// ============================================================================

// HDMA table format:
// - Direct mode: [scanline_count] [data...]
// - Indirect mode: [scanline_count] [address_lo] [address_hi]
// - End marker: 0x00

// Enable HDMA channels (bitmask)
inline void hdma_enable(u8 channel_mask) {
    hal::write8(reg::HDMAEN::address, channel_mask);
}

// Disable all HDMA
inline void hdma_disable() {
    hal::write8(reg::HDMAEN::address, 0);
}

// Set up HDMA channel for direct mode
// channel: HDMA channel (0-7)
// dest: B-bus register ($21xx low byte)
// table: Pointer to HDMA table in CPU memory
// mode_flags: Additional mode flags (use mode:: constants)
template<u8 Channel>
inline void hdma_setup_direct(u8 dest, const void* table, u8 mode_flags = mode::BYTE_TO_ONE) {
    static_assert(Channel < 8, "HDMA channel must be 0-7");

    // Configure HDMA
    set_control<Channel>(mode_flags | addr::INCREMENT);
    set_dest<Channel>(dest);
    set_source<Channel>(reinterpret_cast<u32>(table));
}

// Set up HDMA channel for indirect mode
template<u8 Channel>
inline void hdma_setup_indirect(u8 dest, const void* table, u8 mode_flags = mode::BYTE_TO_ONE) {
    static_assert(Channel < 8, "HDMA channel must be 0-7");

    // Configure HDMA with indirect bit
    set_control<Channel>(mode_flags | addr::INCREMENT | 0x40);  // Bit 6 = indirect
    set_dest<Channel>(dest);
    set_source<Channel>(reinterpret_cast<u32>(table));
}

// ============================================================================
// Convenience Functions
// ============================================================================

// Fill VRAM with a single word value
// Note: This requires a small buffer containing the value to repeat
template<u8 Channel = 0>
inline void fill_vram(u16 vram_addr, const u16* value_ptr, u16 word_count) {
    static_assert(Channel < 8, "DMA channel must be 0-7");

    // Set VRAM address
    hal::write8(reg::VMAIN::address, 0x80);
    hal::write8(reg::VMADDL::address, static_cast<u8>(vram_addr & 0xFF));
    hal::write8(reg::VMADDH::address, static_cast<u8>(vram_addr >> 8));

    // Use fixed source address mode
    set_control<Channel>(mode::WORD_TO_TWO | addr::FIXED | dir::TO_PPU);
    set_dest<Channel>(0x18);  // VMDATAL
    set_source<Channel>(reinterpret_cast<u32>(value_ptr));
    set_size<Channel>(static_cast<u16>(word_count * 2));

    start(static_cast<u8>(1 << Channel));
}

// Upload tiles to VRAM (alias for transfer_to_vram)
template<u8 Channel = 0>
inline void upload_tiles(const void* tiles, u16 vram_addr, u16 size) {
    transfer_to_vram<Channel>(tiles, vram_addr, size);
}

// Upload tilemap to VRAM (alias for transfer_to_vram)
template<u8 Channel = 0>
inline void upload_tilemap(const void* tilemap, u16 vram_addr, u16 size) {
    transfer_to_vram<Channel>(tilemap, vram_addr, size);
}

// Upload palette (alias for transfer_to_cgram)
template<u8 Channel = 0>
inline void upload_palette(const void* palette, u8 start_color, u8 color_count) {
    transfer_to_cgram<Channel>(palette, start_color, static_cast<u16>(color_count * 2));
}

} // namespace snes::dma
