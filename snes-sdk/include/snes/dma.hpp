#pragma once

#include "types.hpp"
#include "hal.hpp"
#include "registers.hpp"

namespace snes::dma {

// DMA channel selection (0-7)
enum class Channel : u8 {
    Ch0 = 0,
    Ch1 = 1,
    Ch2 = 2,
    Ch3 = 3,
    Ch4 = 4,
    Ch5 = 5,
    Ch6 = 6,
    Ch7 = 7
};

// DMA transfer modes
namespace mode {
    constexpr u8 BYTE_TO_SINGLE = 0x00;  // 1 byte to 1 register
    constexpr u8 WORD_TO_PAIR   = 0x01;  // 2 bytes to 2 registers (low, high)
    constexpr u8 BYTE_TO_SAME   = 0x02;  // 2 bytes to same register
    constexpr u8 WORD_TO_TWO    = 0x03;  // 4 bytes to 2 registers (AA, BB)
    constexpr u8 BYTE_TO_FOUR   = 0x04;  // 4 bytes to 4 registers

    constexpr u8 INC_ADDR       = 0x00;  // Increment A-bus address
    constexpr u8 DEC_ADDR       = 0x10;  // Decrement A-bus address
    constexpr u8 FIXED_ADDR     = 0x08;  // Fixed A-bus address

    constexpr u8 A_TO_B         = 0x00;  // Transfer from A-bus to B-bus (CPU to PPU)
    constexpr u8 B_TO_A         = 0x80;  // Transfer from B-bus to A-bus (PPU to CPU)
}

// B-bus destination registers (low byte of $21xx)
namespace dest {
    constexpr u8 VRAM     = 0x18;  // VMDATAL ($2118)
    constexpr u8 VRAM_HI  = 0x19;  // VMDATAH ($2119)
    constexpr u8 OAM      = 0x04;  // OAMDATA ($2104)
    constexpr u8 CGRAM    = 0x22;  // CGDATA ($2122)
}

// Transfer data to VRAM
// vram_addr: Word address in VRAM (0-0x7FFF)
// src: Source data pointer (must be in bank 0-7F or 80-FF)
// size: Number of bytes to transfer
void to_vram(Channel ch, u16 vram_addr, const void* src, u16 size);

// Transfer data to CGRAM (palette)
// start_color: Starting color index (0-255)
// src: Source data pointer (BGR555 format, 2 bytes per color)
// size: Number of bytes to transfer (2 * num_colors)
void to_cgram(Channel ch, u8 start_color, const void* src, u16 size);

// Transfer data to OAM (sprites)
// src: Source data pointer (544 bytes for full OAM)
// size: Number of bytes to transfer (typically 512 for low table + 32 for high table)
void to_oam(Channel ch, const void* src, u16 size);

// Transfer data to OAM at specific offset
void to_oam_at(Channel ch, u16 oam_addr, const void* src, u16 size);

// Generic DMA transfer setup
// ch: DMA channel to use
// ctrl: Control byte (direction, mode, increment)
// dest: B-bus destination register ($21xx low byte)
// src: Source address (24-bit: bank in bits 16-23)
// size: Number of bytes
void transfer(Channel ch, u8 ctrl, u8 dest, u32 src, u16 size);

// Start DMA on specified channels (bitmask)
void start(u8 channel_mask);

// HDMA class for scanline effects
class HdmaChannel {
    u8 m_channel;

public:
    explicit HdmaChannel(Channel ch) : m_channel(static_cast<u8>(ch)) {}

    // Setup HDMA transfer
    // target_reg: B-bus register to write ($21xx low byte)
    // table: Pointer to HDMA table
    // mode: Transfer mode (use mode:: constants)
    void setup(u8 target_reg, const void* table, u8 mode = mode::BYTE_TO_SINGLE);

    // Enable this HDMA channel
    void enable();

    // Disable this HDMA channel
    void disable();

    // Get channel number (0-7)
    u8 channel() const { return m_channel; }
};

// Enable HDMA channels (bitmask)
void hdma_enable(u8 channel_mask);

// Disable HDMA channels (bitmask)
void hdma_disable(u8 channel_mask);

// Disable all HDMA
void hdma_disable_all();

} // namespace snes::dma
