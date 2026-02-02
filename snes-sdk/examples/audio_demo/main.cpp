// SNES SDK Audio Demo
// ====================
// Demonstrates the audio driver API
//
// Controls:
//   D-pad Up/Down: Change volume
//   A: Play beep sound
//   B: Play click sound
//   X: Play confirm sound
//   Y: Play cancel sound
//   L: Decrease music track
//   R: Increase music track
//   Start: Toggle music playback
//   Select: Stop all audio

#include <snes/ppu.hpp>
#include <snes/input.hpp>
#include <snes/audio.hpp>

using namespace snes;
using namespace snes::ppu;
using namespace snes::input;
using namespace snes::audio;

// Demo state
static volatile u8 volume_level;
static volatile u8 music_track;
static volatile u8 music_playing;
static volatile u8 prev_buttons_lo;
static volatile u8 prev_buttons_hi;

// Constants
static constexpr u8 VOLUME_STEP = 16;
static constexpr u8 MAX_VOLUME = 127;
static constexpr u8 MAX_TRACK = 3;

// Helper: Check if button was just pressed (edge detection)
static u8 button_pressed_hi(u8 current, u8 button) {
    return (current & button) && !(prev_buttons_hi & button);
}

static u8 button_pressed_lo(u8 current, u8 button) {
    return (current & button) && !(prev_buttons_lo & button);
}

static void update_display() {
    // Update volume indicator (use sprite Y position)
    // Volume 0-127 maps to Y position 192-65 (inverted, bar goes up)
    u8 bar_y = 192 - (volume_level * 127 / MAX_VOLUME);

    set_oamaddr(0);
    write_oamdata(32);          // X position
    write_oamdata(bar_y);       // Y position (volume indicator)
    write_oamdata(0);           // Tile 0
    write_oamdata(0x30);        // Palette 3, priority 0

    // Track indicator sprite
    write_oamdata(64);          // X position
    write_oamdata(112);         // Y position (center)
    write_oamdata(music_track); // Tile = track number (visual indicator)
    write_oamdata(0x32);        // Palette 3, priority 0, H-flip if playing

    // Playing indicator (flashing sprite)
    if (music_playing) {
        write_oamdata(80);      // X position
        write_oamdata(112);     // Y position
        write_oamdata(1);       // Tile 1 (play symbol)
        write_oamdata(0x30);    // Visible
    } else {
        write_oamdata(0);       // X position (offscreen)
        write_oamdata(240);     // Y position (offscreen)
        write_oamdata(0);
        write_oamdata(0);
    }
}

static void process_input() {
    wait_for_joypad();
    u8 hi = read_joy1h();
    u8 lo = read_joy1l();

    // Volume control (D-pad)
    if (button_pressed_hi(hi, BTN_UP)) {
        if (volume_level < MAX_VOLUME - VOLUME_STEP) {
            volume_level = volume_level + VOLUME_STEP;
        } else {
            volume_level = MAX_VOLUME;
        }
        set_master_volume(volume_level);
    }
    if (button_pressed_hi(hi, BTN_DOWN)) {
        if (volume_level > VOLUME_STEP) {
            volume_level = volume_level - VOLUME_STEP;
        } else {
            volume_level = 0;
        }
        set_master_volume(volume_level);
    }

    // Sound effects (face buttons)
    if (button_pressed_lo(lo, BTN_A)) {
        play_sfx(SFX_BEEP);
    }
    if (button_pressed_lo(lo, BTN_B)) {
        play_sfx(SFX_CLICK);
    }
    if (button_pressed_lo(lo, BTN_X)) {
        play_sfx(SFX_CONFIRM);
    }
    if (button_pressed_lo(lo, BTN_Y)) {
        play_sfx(SFX_CANCEL);
    }

    // Music track selection (L/R)
    if (button_pressed_lo(lo, BTN_L)) {
        if (music_track > 0) {
            music_track = music_track - 1;
        }
    }
    if (button_pressed_lo(lo, BTN_R)) {
        if (music_track < MAX_TRACK) {
            music_track = music_track + 1;
        }
    }

    // Music playback (Start)
    if (button_pressed_lo(lo, BTN_START)) {
        if (music_playing) {
            stop_music();
            music_playing = 0;
        } else {
            play_music(music_track);
            music_playing = 1;
        }
    }

    // Stop all (Select)
    if (button_pressed_lo(lo, BTN_SELECT)) {
        stop_all();
        music_playing = 0;
    }

    // Store previous button state for edge detection
    prev_buttons_hi = hi;
    prev_buttons_lo = lo;
}

int main() {
    static constexpr u8 FULL_BRIGHTNESS = 15;

    // Initialize state
    volume_level = 127;  // Start at max volume
    music_track = 1;     // Start with track 1
    music_playing = 0;
    prev_buttons_hi = 0;
    prev_buttons_lo = 0;

    // Initialize audio system
    // Note: This will wait for the SPC700 driver to be ready
    // If no driver is loaded, init() will timeout and return false
    bool audio_ok = init();

    // Set initial volume if audio initialized
    if (audio_ok) {
        set_master_volume(volume_level);
    }

    enable_joypad();
    screen_on(FULL_BRIGHTNESS);

    // Enable sprites
    set_tm(SCREEN_BG1 | SCREEN_OBJ);

    // Main loop
    for (;;) {
        wait_vblank();
        process_input();
        update_display();
    }

    return 0;
}
