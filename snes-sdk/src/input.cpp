#include <snes/input.hpp>

namespace snes::input {

void init() {
    // Enable joypad auto-read
    u8 nmitimen = hal::read8(reg::NMITIMEN::address);
    hal::write8(reg::NMITIMEN::address, nmitimen | nmi::JOYPAD_ENABLE);
}

void wait_for_read() {
    // Wait for auto-read to complete (bit 0 of HVBJOY clears when done)
    while (hal::read8(reg::HVBJOY::address) & 0x01) {
        // Spin
    }
}

bool is_reading() {
    return (hal::read8(reg::HVBJOY::address) & 0x01) != 0;
}

u16 read_raw(u8 id) {
    // Joypad registers are at $4218-$421F (2 bytes each)
    u32 addr_lo = 0x4218 + (id * 2);
    u8 lo = hal::read8(addr_lo);
    u8 hi = hal::read8(addr_lo + 1);
    return static_cast<u16>(lo | (hi << 8));
}

void Joypad::update() {
    m_previous = m_current;
    m_current = read_raw(m_id);
}

Direction Joypad::direction() const {
    bool up = held(Button::Up);
    bool down = held(Button::Down);
    bool left = held(Button::Left);
    bool right = held(Button::Right);

    // Handle diagonals
    if (up && left) return Direction::UpLeft;
    if (up && right) return Direction::UpRight;
    if (down && left) return Direction::DownLeft;
    if (down && right) return Direction::DownRight;

    // Handle cardinals
    if (up) return Direction::Up;
    if (down) return Direction::Down;
    if (left) return Direction::Left;
    if (right) return Direction::Right;

    return Direction::None;
}

} // namespace snes::input
