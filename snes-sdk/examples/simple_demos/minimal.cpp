// SNES SDK Example: Minimal Demo
// A simple demo that avoids register pressure issues

#include <snes/types.hpp>
#include <snes/hal.hpp>
#include <snes/registers.hpp>
#include <snes/ppu.hpp>
#include <snes/input.hpp>

using namespace snes;

// Simple sprite position update - avoid complex class methods
static void update_sprite(u8 id, i16 x, u8 y) {
    ppu::oam_low[id].x_low = static_cast<u8>(x & 0xFF);
    ppu::oam_low[id].y = y;

    // Set X high bit
    u8 byte_idx = id >> 2;
    u8 bit_pos = (id & 0x03) << 1;
    if (x & 0x100) {
        ppu::oam_high[byte_idx] |= static_cast<u8>(1 << bit_pos);
    } else {
        ppu::oam_high[byte_idx] &= static_cast<u8>(~(1 << bit_pos));
    }
}

int main() {
    // Force blank during setup
    ppu::screen_off();

    // Clear all sprites
    ppu::sprites_clear();

    // Set sprite 0 tile
    ppu::oam_low[0].tile = 0;
    ppu::oam_low[0].attr = 0;

    // Enable sprites on main screen
    ppu::set_tm(ppu::SCREEN_OBJ);

    // Set background color (dark blue)
    ppu::set_bgcolor(Color::from_rgb(0, 0, 8));

    // Enable joypad
    input::enable_joypad();

    // Turn on screen
    ppu::screen_on(15);

    // Player position
    i16 x = 128;
    i16 y = 112;

    // Previous joypad state for edge detection
    u16 prev_joy = 0;

    // Main loop
    for (;;) {
        // Wait for vblank
        ppu::wait_vblank();

        // Wait for joypad auto-read
        input::wait_for_joypad();

        // Read joypad
        u16 joy = input::read_joy1();

        // Move with d-pad
        if (joy & input::BTN16_LEFT)  x -= 2;
        if (joy & input::BTN16_RIGHT) x += 2;
        if (joy & input::BTN16_UP)    y -= 2;
        if (joy & input::BTN16_DOWN)  y += 2;

        // Clamp position
        if (x < 0) x = 0;
        if (x > 248) x = 248;
        if (y < 0) y = 0;
        if (y > 216) y = 216;

        // Update sprite
        update_sprite(0, x, static_cast<u8>(y));

        // Upload to OAM
        ppu::sprites_upload();

        // Store previous state
        prev_joy = joy;
    }

    return 0;
}
