#pragma once

// SNES Input API - Header-only input functions for W65816 backend
// All functions are inline single register reads/writes to avoid register pressure

#include "types.hpp"
#include "hal.hpp"
#include "registers.hpp"

namespace snes::input {

// ============================================================================
// Joypad Reading
// ============================================================================

// Read joypad 1 low byte (A, X, L, R buttons)
inline u8 read_joy1l() { return hal::read8(reg::JOY1L::address); }

// Read joypad 1 high byte (B, Y, Select, Start, D-pad)
inline u8 read_joy1h() { return hal::read8(reg::JOY1H::address); }

// Read full 16-bit joypad 1 state
inline u16 read_joy1() {
    u8 lo = hal::read8(reg::JOY1L::address);
    u8 hi = hal::read8(reg::JOY1H::address);
    return static_cast<u16>((hi << 8) | lo);
}

// Read joypad 2 low byte
inline u8 read_joy2l() { return hal::read8(reg::JOY2L::address); }

// Read joypad 2 high byte
inline u8 read_joy2h() { return hal::read8(reg::JOY2H::address); }

// Read full 16-bit joypad 2 state
inline u16 read_joy2() {
    u8 lo = hal::read8(reg::JOY2L::address);
    u8 hi = hal::read8(reg::JOY2H::address);
    return static_cast<u16>((hi << 8) | lo);
}

// ============================================================================
// Joypad Control
// ============================================================================

// Enable joypad auto-read (reads during VBlank)
inline void enable_joypad() {
    hal::write8(reg::NMITIMEN::address, nmi::JOYPAD_ENABLE);
}

// Wait for joypad auto-read to complete
// Must call after VBlank before reading joypad registers
inline void wait_for_joypad() {
    // Simple busy wait - check bit 0 of HVBJOY
    volatile u8* hvbjoy = reinterpret_cast<volatile u8*>(0x4212);
    while (*hvbjoy & 0x01) {}
}

// ============================================================================
// Button Masks - High Byte (joy1h / joy2h)
// ============================================================================

static constexpr u8 BTN_B      = 0x80;  // B button
static constexpr u8 BTN_Y      = 0x40;  // Y button
static constexpr u8 BTN_SELECT = 0x20;  // Select button
static constexpr u8 BTN_START  = 0x10;  // Start button
static constexpr u8 BTN_UP     = 0x08;  // D-pad up
static constexpr u8 BTN_DOWN   = 0x04;  // D-pad down
static constexpr u8 BTN_LEFT   = 0x02;  // D-pad left
static constexpr u8 BTN_RIGHT  = 0x01;  // D-pad right

// ============================================================================
// Button Masks - Low Byte (joy1l / joy2l)
// ============================================================================

static constexpr u8 BTN_A      = 0x80;  // A button
static constexpr u8 BTN_X      = 0x40;  // X button
static constexpr u8 BTN_L      = 0x20;  // L shoulder
static constexpr u8 BTN_R      = 0x10;  // R shoulder

// ============================================================================
// 16-bit Button Masks (for use with read_joy1() / read_joy2())
// ============================================================================

static constexpr u16 BTN16_B      = 0x8000;
static constexpr u16 BTN16_Y      = 0x4000;
static constexpr u16 BTN16_SELECT = 0x2000;
static constexpr u16 BTN16_START  = 0x1000;
static constexpr u16 BTN16_UP     = 0x0800;
static constexpr u16 BTN16_DOWN   = 0x0400;
static constexpr u16 BTN16_LEFT   = 0x0200;
static constexpr u16 BTN16_RIGHT  = 0x0100;
static constexpr u16 BTN16_A      = 0x0080;
static constexpr u16 BTN16_X      = 0x0040;
static constexpr u16 BTN16_L      = 0x0020;
static constexpr u16 BTN16_R      = 0x0010;

} // namespace snes::input
