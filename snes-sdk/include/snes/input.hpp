#pragma once

#include "types.hpp"
#include "hal.hpp"
#include "registers.hpp"

namespace snes::input {

// Button constants (strongly typed enum for type safety)
enum class Button : u16 {
    A      = 0x0080,
    B      = 0x8000,
    X      = 0x0040,
    Y      = 0x4000,
    L      = 0x0020,
    R      = 0x0010,
    Start  = 0x1000,
    Select = 0x2000,
    Up     = 0x0800,
    Down   = 0x0400,
    Left   = 0x0200,
    Right  = 0x0100
};

// Enable bitwise operations on Button
constexpr Button operator|(Button a, Button b) {
    return static_cast<Button>(static_cast<u16>(a) | static_cast<u16>(b));
}

constexpr u16 operator&(u16 state, Button b) {
    return state & static_cast<u16>(b);
}

constexpr bool has_button(u16 state, Button b) {
    return (state & static_cast<u16>(b)) != 0;
}

// D-pad direction
enum class Direction : u8 {
    None  = 0,
    Up    = 1,
    Down  = 2,
    Left  = 3,
    Right = 4,
    UpLeft    = 5,
    UpRight   = 6,
    DownLeft  = 7,
    DownRight = 8
};

// Joypad class - manages state for one controller
class Joypad {
    u8 m_id;
    u16 m_current;
    u16 m_previous;

public:
    explicit Joypad(u8 id) : m_id(id), m_current(0), m_previous(0) {}

    // Call once per frame after vblank to update state
    void update();

    // Raw button state (all bits)
    u16 raw() const { return m_current; }

    // Check if button is currently held
    bool held(Button b) const {
        return has_button(m_current, b);
    }

    // Check if button was just pressed this frame
    bool pressed(Button b) const {
        return has_button(m_current, b) && !has_button(m_previous, b);
    }

    // Check if button was just released this frame
    bool released(Button b) const {
        return !has_button(m_current, b) && has_button(m_previous, b);
    }

    // Check multiple buttons at once (all must be held)
    bool held_all(u16 buttons) const {
        return (m_current & buttons) == buttons;
    }

    // Check multiple buttons at once (any must be held)
    bool held_any(u16 buttons) const {
        return (m_current & buttons) != 0;
    }

    // Get d-pad direction
    Direction direction() const;

    // Get raw X axis (-1 = left, 0 = center, 1 = right)
    i8 axis_x() const {
        if (held(Button::Left)) return -1;
        if (held(Button::Right)) return 1;
        return 0;
    }

    // Get raw Y axis (-1 = up, 0 = center, 1 = down)
    i8 axis_y() const {
        if (held(Button::Up)) return -1;
        if (held(Button::Down)) return 1;
        return 0;
    }

    // Controller ID (0-3)
    u8 id() const { return m_id; }
};

// Global functions

// Initialize input system (enables joypad auto-read)
void init();

// Wait for auto-read to complete (call after vblank)
void wait_for_read();

// Check if auto-read is in progress
bool is_reading();

// Read raw joypad data directly (bypasses Joypad class)
u16 read_raw(u8 id);

} // namespace snes::input
