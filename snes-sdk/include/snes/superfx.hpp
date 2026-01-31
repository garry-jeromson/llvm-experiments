#pragma once

#include "types.hpp"

namespace snes::superfx {

// SuperFX (GSU) Memory Map:
// $3000-$32FF: GSU registers
// $6000-$7FFF: RAM (cartridge)
// $8000-$FFFF: ROM (bank-switched)

// GSU register addresses
namespace reg {
    // Status/Control
    constexpr u32 SFR    = 0x3030;  // Status/Flag Register (16-bit)
    constexpr u32 PBR    = 0x3034;  // Program Bank Register
    constexpr u32 ROMBR  = 0x3036;  // ROM Bank Register
    constexpr u32 RAMBR  = 0x303C;  // RAM Bank Register
    constexpr u32 CBR    = 0x303E;  // Cache Base Register (16-bit)

    // Screen registers
    constexpr u32 SCBR   = 0x3038;  // Screen Base Register
    constexpr u32 SCMR   = 0x303A;  // Screen Mode Register

    // Plot registers
    constexpr u32 CLSR   = 0x3039;  // Clock Speed Register
    constexpr u32 POR    = 0x303B;  // Plot Option Register

    // Config
    constexpr u32 CFGR   = 0x3037;  // Config Register
    constexpr u32 BRAMR  = 0x0033;  // Backup RAM Register
}

// SFR (Status Flag Register) bits
namespace sfr {
    constexpr u16 GO     = 0x0020;  // GSU running
    constexpr u16 IRQ    = 0x8000;  // IRQ pending
    constexpr u16 ALT1   = 0x0100;  // ALT1 flag
    constexpr u16 ALT2   = 0x0200;  // ALT2 flag
    constexpr u16 IL     = 0x0400;  // Immediate low byte
    constexpr u16 IH     = 0x0800;  // Immediate high byte
    constexpr u16 B      = 0x1000;  // WITH flag
    constexpr u16 CY     = 0x0004;  // Carry
    constexpr u16 S      = 0x0002;  // Sign
    constexpr u16 Z      = 0x0001;  // Zero
    constexpr u16 OV     = 0x0008;  // Overflow
}

// SCMR (Screen Mode Register) values
namespace scmr {
    constexpr u8 MODE_2BPP  = 0x00;
    constexpr u8 MODE_4BPP  = 0x01;
    constexpr u8 MODE_8BPP  = 0x03;
    constexpr u8 HEIGHT_128 = 0x00;
    constexpr u8 HEIGHT_160 = 0x04;
    constexpr u8 HEIGHT_192 = 0x08;
    constexpr u8 HEIGHT_OBJ = 0x0C;  // Object mode
}

// POR (Plot Option Register) bits
namespace por {
    constexpr u8 TRANSPARENT = 0x01;  // Plot transparent pixels
    constexpr u8 DITHER      = 0x02;  // Enable dithering
    constexpr u8 HIGH_NIBBLE = 0x04;  // Plot high nibble (4bpp)
    constexpr u8 FREEZE_HIGH = 0x08;  // Freeze high byte (for PLOT)
    constexpr u8 OBJ_MODE    = 0x10;  // Object/sprite mode
}

// Initialize SuperFX chip
void init();

// Check if SuperFX is present (returns false on non-SuperFX carts)
bool detect();

// Upload code/data to GSU RAM
// addr: Destination address in GSU RAM space
// data: Source data
// size: Number of bytes
void upload(u16 addr, const void* data, u16 size);

// Start GSU execution at specified address
void run(u16 addr);

// Wait for GSU to finish executing
void wait();

// Check if GSU is currently running
bool busy();

// Stop GSU execution
void stop();

// Register access (R0-R15)
void set_reg(u8 reg_num, u16 value);
u16 get_reg(u8 reg_num);

// Set ROM bank for GSU access
void set_rom_bank(u8 bank);

// Set RAM bank for GSU access
void set_ram_bank(u8 bank);

// Set screen buffer base address (in GSU RAM)
void set_screen_base(u8 page);  // Page = address / 0x400

// Set screen mode
void set_screen_mode(u8 bpp, u8 height_mode);

// Set plot options
void set_plot_options(u8 options);

// Enable/disable GSU IRQ
void enable_irq(bool enable);

// Clear GSU IRQ flag
void clear_irq();

// Read GSU status
u16 get_status();

// Cache control
void set_cache_base(u16 addr);
void flush_cache();

// High-level graphics operations (require appropriate GSU code)

// These functions upload simple GSU routines for common operations
// They're meant for simple use cases - for serious GSU programming,
// upload custom GSU code

// Clear screen buffer to color
void clear_screen(u8 color);

// Draw a filled rectangle
void fill_rect(i16 x, i16 y, u16 width, u16 height, u8 color);

// Draw a line (Bresenham)
void draw_line(i16 x1, i16 y1, i16 x2, i16 y2, u8 color);

// Plot a single pixel
void plot(i16 x, i16 y, u8 color);

// Copy GSU screen buffer to VRAM
// This triggers DMA from GSU RAM to VRAM during vblank
void copy_to_vram(u16 vram_addr, u16 gsu_addr, u16 size);

} // namespace snes::superfx
