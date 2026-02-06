// SNES SDK Example: Hello World
// Demonstrates basic SDK usage with input and sprites

#include <snes/snes.hpp>

using namespace snes;

int main() {
    // Initialize SDK
    snes::init();

    // Set up input
    input::Joypad pad1(0);

    // Set up background 1
    ppu::Background bg1(1);
    bg1.set_tilemap(0x1000);
    bg1.set_tiles(0x2000);
    bg1.enable();

    // Create a player sprite
    ppu::Sprite player(0);
    i16 x = 128;
    i16 y = 112;

    // Set background color (blue)
    ppu::set_bgcolor(0, 0, 15);

    // Turn on the screen
    ppu::screen_on();

    // Main loop
    while (true) {
        // Wait for vblank
        ppu::wait_vblank();

        // Update input
        pad1.update();

        // Move player with d-pad
        if (pad1.held(input::Button::Left))  x -= 2;
        if (pad1.held(input::Button::Right)) x += 2;
        if (pad1.held(input::Button::Up))    y -= 2;
        if (pad1.held(input::Button::Down))  y += 2;

        // Clamp to screen bounds
        x = math::clamp(x, static_cast<i16>(0), static_cast<i16>(255));
        y = math::clamp(y, static_cast<i16>(0), static_cast<i16>(223));

        // Update sprite position
        player.set_pos(x, static_cast<u8>(y));
        player.set_tile(0);  // First sprite tile

        // Upload OAM to PPU
        ppu::sprites_update();
    }

    return 0;
}
