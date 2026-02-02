// SNES SDK SuperFX Demo
// ======================
// Demonstrates the SuperFX coprocessor API
//
// This demo shows:
// - SuperFX detection and initialization
// - GSU program upload and execution
// - Frame buffer rendering
//
// Note: Requires a cartridge with SuperFX chip (GSU-1 or GSU-2)
//       or an emulator with SuperFX support.
//
// Controls:
//   A: Run SuperFX fill program
//   B: Copy frame buffer to VRAM
//   X: Toggle high-speed mode (GSU-2 only)
//   Start: Reset

#include <snes/ppu.hpp>
#include <snes/input.hpp>
#include <snes/superfx.hpp>

using namespace snes;
using namespace snes::ppu;
using namespace snes::input;
using namespace snes::superfx;

// Assembly functions for SuperFX control (defined in crt0.s)
// Note: Function names must match the exported symbols
extern "C" {
    void _sfx_upload_and_run();
    u16 _sfx_is_running();
    void _sfx_copy_framebuffer();
}

// Demo state
static volatile u8 demo_running;
static volatile u8 highspeed_enabled;
static volatile u8 prev_buttons_lo;
static volatile u8 sfx_detected;
static volatile u8 frame_ready;

// Helper: Check if button was just pressed
static u8 button_pressed_lo(u8 current, u8 button) {
    return (current & button) && !(prev_buttons_lo & button);
}

static void display_status() {
    // Update display based on demo state
    // Sprite indicates current state

    set_oamaddr(0);
    if (demo_running) {
        // SuperFX is running - sprite at center, flashing
        write_oamdata(120);     // X position (center)
        write_oamdata(100);     // Y position
        write_oamdata(0);       // Tile 0
        write_oamdata(0x30);    // Palette 3 (cyan)
    } else if (frame_ready) {
        // Frame ready to copy - sprite at right
        write_oamdata(200);     // X position (right)
        write_oamdata(100);     // Y position
        write_oamdata(0);       // Tile 0
        write_oamdata(0x32);    // Palette 3, different
    } else {
        // Idle - sprite at left
        write_oamdata(40);      // X position (left)
        write_oamdata(100);     // Y position
        write_oamdata(0);       // Tile 0
        write_oamdata(0x31);    // Palette 3
    }
}

static void process_input() {
    wait_for_joypad();
    u8 lo = read_joy1l();

    if (button_pressed_lo(lo, BTN_A)) {
        // Run SuperFX fill program
        demo_running = 1;
        frame_ready = 0;
        // Upload GSU program to SuperFX RAM and start execution
        _sfx_upload_and_run();
    }

    if (button_pressed_lo(lo, BTN_B)) {
        // Copy SuperFX frame buffer to VRAM (if ready)
        if (frame_ready) {
            _sfx_copy_framebuffer();
        }
        demo_running = 0;
    }

    if (button_pressed_lo(lo, BTN_X)) {
        // Toggle high-speed mode (GSU-2 only)
        if (sfx_detected && get_version() >= 2) {
            if (highspeed_enabled) {
                disable_highspeed();
                highspeed_enabled = 0;
            } else {
                enable_highspeed();
                highspeed_enabled = 1;
            }
        }
    }

    if (button_pressed_lo(lo, BTN_START)) {
        // Reset
        demo_running = 0;
        highspeed_enabled = 0;
        if (sfx_detected) {
            disable_highspeed();
        }
    }

    prev_buttons_lo = lo;
}

int main() {
    static constexpr u8 FULL_BRIGHTNESS = 15;

    // Initialize state
    demo_running = 0;
    highspeed_enabled = 0;
    prev_buttons_lo = 0;
    sfx_detected = 0;
    frame_ready = 0;

    // DEBUG: Skip SuperFX init for now - it reads hardware registers
    // Try to initialize SuperFX
    // Note: This will only succeed if the emulator recognizes this as a SuperFX cart
    // if (init()) {
    //     sfx_detected = 1;
    // }

    enable_joypad();
    screen_on(FULL_BRIGHTNESS);

    // Enable BG1 and sprites
    set_tm(SCREEN_BG1 | SCREEN_OBJ);

    // Main loop
    for (;;) {
        wait_vblank();
        process_input();
        display_status();

        // DEBUG: Skip SuperFX status check for now
        // If SuperFX was running, check if it finished
        // if (demo_running && !_sfx_is_running()) {
        //     // SuperFX program completed - frame buffer is ready
        //     frame_ready = 1;
        //     demo_running = 0;
        // }
    }

    return 0;
}
