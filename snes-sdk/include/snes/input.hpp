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

// ============================================================================
// Button Enum (type-safe, 16-bit values)
// ============================================================================

enum class Button : u16 {
    B      = 0x8000,
    Y      = 0x4000,
    Select = 0x2000,
    Start  = 0x1000,
    Up     = 0x0800,
    Down   = 0x0400,
    Left   = 0x0200,
    Right  = 0x0100,
    A      = 0x0080,
    X      = 0x0040,
    L      = 0x0020,
    R      = 0x0010
};

// Allow bitwise OR on Button values
inline u16 operator|(Button a, Button b) {
    return static_cast<u16>(a) | static_cast<u16>(b);
}

inline u16 operator|(u16 a, Button b) {
    return a | static_cast<u16>(b);
}

// ============================================================================
// Direction Enum (8-way + none)
// ============================================================================

enum class Direction : u8 {
    None       = 0,
    Up         = 1,
    UpRight    = 2,
    Right      = 3,
    DownRight  = 4,
    Down       = 5,
    DownLeft   = 6,
    Left       = 7,
    UpLeft     = 8
};

// ============================================================================
// Joypad Class (stateful input handling with edge detection)
// ============================================================================

class Joypad {
    u8 m_id;          // Joypad index (0 or 1)
    u16 m_current;    // Current frame button state
    u16 m_previous;   // Previous frame button state

public:
    // Construct for joypad 0 or 1
    explicit Joypad(u8 id = 0) : m_id(id), m_current(0), m_previous(0) {}

    // Update button state (call once per frame)
    void update() {
        m_previous = m_current;

        // Read from hardware based on joypad ID
        u32 addr_lo = 0x4218 + (m_id * 2);
        u32 addr_hi = addr_lo + 1;

        u8 lo = hal::read8(addr_lo);
        u8 hi = hal::read8(addr_hi);
        m_current = static_cast<u16>((hi << 8) | lo);
    }

    // Get raw button state
    u16 raw() const { return m_current; }

    // Check if button is currently held
    bool held(Button btn) const {
        return (m_current & static_cast<u16>(btn)) != 0;
    }

    // Check if any of the specified buttons are held
    bool held_any(u16 mask) const {
        return (m_current & mask) != 0;
    }

    // Check if all of the specified buttons are held
    bool held_all(u16 mask) const {
        return (m_current & mask) == mask;
    }

    // Check if button was just pressed this frame (edge detection)
    bool pressed(Button btn) const {
        u16 mask = static_cast<u16>(btn);
        return (m_current & mask) != 0 && (m_previous & mask) == 0;
    }

    // Check if button was just released this frame (edge detection)
    bool released(Button btn) const {
        u16 mask = static_cast<u16>(btn);
        return (m_current & mask) == 0 && (m_previous & mask) != 0;
    }

    // Get current D-pad direction (8-way + none)
    Direction direction() const {
        bool up    = held(Button::Up);
        bool down  = held(Button::Down);
        bool left  = held(Button::Left);
        bool right = held(Button::Right);

        // Cancel out opposing directions
        if (up && down) { up = false; down = false; }
        if (left && right) { left = false; right = false; }

        if (up) {
            if (left) return Direction::UpLeft;
            if (right) return Direction::UpRight;
            return Direction::Up;
        }
        if (down) {
            if (left) return Direction::DownLeft;
            if (right) return Direction::DownRight;
            return Direction::Down;
        }
        if (left) return Direction::Left;
        if (right) return Direction::Right;

        return Direction::None;
    }

    // Get horizontal axis (-1 = left, 0 = none, 1 = right)
    i8 axis_x() const {
        bool left  = held(Button::Left);
        bool right = held(Button::Right);

        if (left && !right) return -1;
        if (right && !left) return 1;
        return 0;
    }

    // Get vertical axis (-1 = up, 0 = none, 1 = down)
    i8 axis_y() const {
        bool up   = held(Button::Up);
        bool down = held(Button::Down);

        if (up && !down) return -1;
        if (down && !up) return 1;
        return 0;
    }
};

} // namespace snes::input
