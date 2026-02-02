#pragma once

// SNES SuperFX API - Interface for the SuperFX RISC coprocessor
//
// The SuperFX (GSU-1/GSU-2) is a RISC coprocessor found in cartridges like
// Star Fox, Stunt Race FX, and Yoshi's Island. It provides:
// - 16-bit RISC processor with custom instruction set
// - 2D/3D graphics acceleration
// - Frame buffer rendering
// - Polygon filling, rotation, scaling
//
// Memory Map:
//   $3000-$301F  - SuperFX control registers
//   $3030-$303B  - SuperFX status and cache
//   $3100-$32FF  - SuperFX program cache (512 bytes)
//
// The SuperFX has its own ROM and RAM:
//   - Program ROM: Up to 2MB (cartridge ROM)
//   - RAM: 32KB-128KB for frame buffer and work memory

#include "types.hpp"
#include "hal.hpp"

namespace snes {
namespace superfx {

// SuperFX register addresses
namespace reg {
    constexpr u16 SFR       = 0x3030;  // Status/Flag Register (16-bit)
    constexpr u16 BRAMR     = 0x3033;  // Backup RAM register
    constexpr u16 PBR       = 0x3034;  // Program Bank Register
    constexpr u16 ROMBR     = 0x3036;  // ROM Bank Register
    constexpr u16 CFGR      = 0x3037;  // Config Register
    constexpr u16 SCBR      = 0x3038;  // Screen Base Register
    constexpr u16 CLSR      = 0x3039;  // Clock Speed Register
    constexpr u16 SCMR      = 0x303A;  // Screen Mode Register
    constexpr u16 VCR       = 0x303B;  // Version Code Register
    constexpr u16 RAMBR     = 0x303C;  // RAM Bank Register
    constexpr u16 CBR       = 0x303E;  // Cache Base Register (16-bit)
}

// SFR (Status/Flag Register) bits
namespace sfr {
    constexpr u16 IRQ       = 0x8000;  // Interrupt pending
    constexpr u16 B         = 0x1000;  // WITH executed flag
    constexpr u16 IH        = 0x0800;  // Immediate upper nibble
    constexpr u16 IL        = 0x0400;  // Immediate lower nibble
    constexpr u16 ALT2      = 0x0200;  // ALT2 mode
    constexpr u16 ALT1      = 0x0100;  // ALT1 mode
    constexpr u16 R         = 0x0040;  // Reading ROM flag
    constexpr u16 GO        = 0x0020;  // Go/Running flag
    constexpr u16 OV        = 0x0010;  // Overflow
    constexpr u16 S         = 0x0008;  // Sign
    constexpr u16 CY        = 0x0004;  // Carry
    constexpr u16 Z         = 0x0002;  // Zero
}

// CFGR (Config Register) bits
namespace cfgr {
    constexpr u8 IRQ        = 0x80;    // IRQ enable
    constexpr u8 MS0        = 0x20;    // Multiplier speed (0=standard, 1=high)
}

// SCMR (Screen Mode Register) bits
namespace scmr {
    constexpr u8 HT_MASK    = 0x24;    // Height mask (192/160/128 lines)
    constexpr u8 HT_128     = 0x00;    // 128 lines
    constexpr u8 HT_160     = 0x04;    // 160 lines
    constexpr u8 HT_192     = 0x20;    // 192 lines
    constexpr u8 HT_OBJ     = 0x24;    // OBJ mode
    constexpr u8 RON        = 0x10;    // ROM access enable
    constexpr u8 RAN        = 0x08;    // RAM access enable
    constexpr u8 MD_MASK    = 0x03;    // Color depth mask
    constexpr u8 MD_2BPP    = 0x00;    // 2bpp (4 colors)
    constexpr u8 MD_4BPP    = 0x01;    // 4bpp (16 colors)
    constexpr u8 MD_8BPP    = 0x03;    // 8bpp (256 colors)
}

// CLSR (Clock Speed Register)
namespace clsr {
    constexpr u8 SPEED_STD  = 0x00;    // Standard speed (10.7 MHz)
    constexpr u8 SPEED_HIGH = 0x01;    // High speed (21.4 MHz)
}

// Global SuperFX state (defined in crt0.s or user code)
extern volatile u8 g_sfx_initialized;
extern volatile u8 g_sfx_version;

// ============================================================================
// Register Access (declare early for use by other functions)
// ============================================================================

// Read Status/Flag Register
inline u16 read_sfr() {
    u8 lo = hal::read8(reg::SFR);
    u8 hi = hal::read8(reg::SFR + 1);
    return static_cast<u16>(lo | (hi << 8));
}

// Write to Config Register
inline void write_cfgr(u8 val) {
    hal::write8(reg::CFGR, val);
}

// Write to Screen Base Register (frame buffer address)
inline void write_scbr(u8 val) {
    hal::write8(reg::SCBR, val);
}

// Write to Clock Speed Register
inline void write_clsr(u8 val) {
    hal::write8(reg::CLSR, val);
}

// Write to Screen Mode Register
inline void write_scmr(u8 val) {
    hal::write8(reg::SCMR, val);
}

// Write to Program Bank Register (start execution address)
inline void write_pbr(u8 val) {
    hal::write8(reg::PBR, val);
}

// Read Version Code Register
inline u8 read_vcr() {
    return hal::read8(reg::VCR);
}

// ============================================================================
// Initialization
// ============================================================================

// Initialize the SuperFX coprocessor
// Returns true if SuperFX is present and initialized
inline bool init() {
    // Read version register to detect SuperFX
    u8 vcr = read_vcr();
    // GSU-1 returns $01, GSU-2 returns $04
    if (vcr == 0x01) {
        g_sfx_version = 1;
        g_sfx_initialized = 1;
        return true;
    } else if (vcr == 0x04) {
        g_sfx_version = 2;
        g_sfx_initialized = 1;
        return true;
    }
    g_sfx_initialized = 0;
    return false;
}

// Check if SuperFX is present
inline bool is_present() {
    return g_sfx_initialized != 0;
}

// Get SuperFX version (1 for GSU-1, 2 for GSU-2)
inline u8 get_version() {
    return g_sfx_version;
}

// ============================================================================
// Execution Control
// ============================================================================

// Check if SuperFX is currently running
inline bool is_running() {
    return (read_sfr() & sfr::GO) != 0;
}

// Wait for SuperFX to finish execution
inline void wait_done() {
    while (is_running()) {
        // Spin wait
    }
}

// Stop SuperFX execution (just update tracking state)
inline void stop() {
    // GSU stops automatically when STOP instruction is executed
}

// ============================================================================
// Frame Buffer Setup
// ============================================================================

// Screen height options (scanlines)
constexpr u8 HEIGHT_128 = 128;
constexpr u8 HEIGHT_160 = 160;
constexpr u8 HEIGHT_192 = 192;

// Color depth options (bits per pixel)
constexpr u8 DEPTH_2BPP = 2;   // 4 colors
constexpr u8 DEPTH_4BPP = 4;   // 16 colors (default)
constexpr u8 DEPTH_8BPP = 8;   // 256 colors

// Configure screen mode (height and color depth)
// ram_bank: SuperFX RAM bank (0 or 1)
// height:   Frame buffer height (HEIGHT_128, HEIGHT_160, or HEIGHT_192)
// depth:    Color depth in bits (DEPTH_2BPP, DEPTH_4BPP, or DEPTH_8BPP)
inline void configure_screen(u8 ram_bank, u8 height, u8 depth) {
    u8 mode = 0;
    // Set color depth
    if (depth == DEPTH_2BPP) mode |= scmr::MD_2BPP;
    else if (depth == DEPTH_8BPP) mode |= scmr::MD_8BPP;
    else mode |= scmr::MD_4BPP;  // Default to 4bpp
    // Set height
    if (height == HEIGHT_160) mode |= scmr::HT_160;
    else if (height == HEIGHT_192) mode |= scmr::HT_192;
    else mode |= scmr::HT_128;  // Default to 128 lines
    // Enable ROM and RAM access
    mode |= scmr::RON | scmr::RAN;
    // Set RAM bank
    hal::write8(reg::RAMBR, ram_bank);
    // Apply mode
    write_scmr(mode);
}

// Set frame buffer base address
// The address is (scbr << 10) in SuperFX RAM
inline void set_framebuffer(u8 scbr) {
    write_scbr(scbr);
}

// ============================================================================
// High-Speed Mode (GSU-2 only)
// ============================================================================

// Enable high-speed mode (21.4 MHz)
// Only available on GSU-2
inline void enable_highspeed() {
    write_clsr(clsr::SPEED_HIGH);
    write_cfgr(cfgr::MS0);  // Fast multiplier
}

// Disable high-speed mode (10.7 MHz)
inline void disable_highspeed() {
    write_clsr(clsr::SPEED_STD);
    write_cfgr(0);
}

// ============================================================================
// Interrupt Handling
// ============================================================================

// Enable SuperFX IRQ
inline void enable_irq() {
    u8 cfg = cfgr::IRQ;
    write_cfgr(cfg);
}

// Disable SuperFX IRQ
inline void disable_irq() {
    write_cfgr(0);
}

// Check if IRQ is pending
inline bool irq_pending() {
    return (read_sfr() & sfr::IRQ) != 0;
}

// Acknowledge IRQ by reading SFR
inline void ack_irq() {
    (void)read_sfr();
}

// ============================================================================
// Memory Access (bank 0 only, 16-bit addresses)
// ============================================================================

// Note: Full 24-bit addressing requires assembly routines.
// These functions access SuperFX RAM at $70:0000-$70:FFFF (first 64KB only)

// Write to SuperFX RAM at offset (bank $70)
inline void write_ram_lo(u16 addr, u8 val) {
    // Use long addressing via inline assembly if needed
    // For now, this requires the Data Bank to be set to $70
    // hal::write8(0x700000 | addr, val); // Would need 24-bit support
}

// Read from SuperFX RAM at offset (bank $70)
inline u8 read_ram_lo(u16 addr) {
    // Would need 24-bit addressing support
    return 0;
}

// NOTE: upload_and_run() removed - requires 24-bit addressing support.
// To upload SuperFX programs, use DMA or assembly routines with long addressing.

} // namespace superfx
} // namespace snes
