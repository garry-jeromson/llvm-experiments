// SDK Lite Demo - Minimal sprite demo for W65816 backend
// Uses direct register writes like bounce_demo

#include <snes/ppu.hpp>
#include <snes/input.hpp>

using namespace snes;
using namespace snes::ppu;
using namespace snes::input;

// Sprite position
static u8 sprite_x;
static u8 sprite_y;

// Write sprite 0 directly to OAM hardware
static void write_oam() {
    set_oamaddl(0);
    set_oamaddh(0);
    hal::write8(0x2104, sprite_x);  // X position
    hal::write8(0x2104, sprite_y);  // Y position
    hal::write8(0x2104, 0);         // Tile 0
    hal::write8(0x2104, 0x30);      // Priority 3, palette 0
}

static void handle_input() {
    wait_for_joypad();
    u8 btns = read_joy1h();

    if (btns & BTN_LEFT) {
        if (sprite_x > 0) --sprite_x;
    }
    if (btns & BTN_RIGHT) {
        if (sprite_x < 248) ++sprite_x;
    }
    if (btns & BTN_UP) {
        if (sprite_y > 0) --sprite_y;
    }
    if (btns & BTN_DOWN) {
        if (sprite_y < 216) ++sprite_y;
    }
}

int main() {
    // Initialize position
    sprite_x = 128;
    sprite_y = 112;

    // Dark blue background
    set_bgcolor_lo(0x00);
    set_bgcolor_hi(0x40);

    // Enable joypad auto-read
    enable_joypad();

    // Turn screen on
    screen_on(15);

    // Main loop
    for (;;) {
        wait_vblank();
        write_oam();
        handle_input();
    }

    return 0;
}
