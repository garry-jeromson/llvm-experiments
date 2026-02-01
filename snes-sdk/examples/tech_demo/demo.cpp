// Parallax Scroller Demo - SNES SDK Showcase
// Features:
//   - Dual-layer parallax scrolling backgrounds
//   - D-pad controlled player sprite
//   - Smooth animation
//
// Uses cleaned-up SDK with proper register names

#include <snes/ppu.hpp>
#include <snes/input.hpp>

using namespace snes;
using namespace snes::ppu;
using namespace snes::input;

// Player state
static u8 player_x;
static u8 player_y;

// Scroll positions (16-bit for sub-pixel precision)
// Use volatile to prevent compiler from keeping in registers incorrectly
static volatile u16 scroll_bg1;
static volatile u16 scroll_bg2;

// Frame counter for animation
static u8 frame;

// Write player sprite to OAM
static void update_oam() {
    set_oamaddr(0);
    write_oamdata(player_x);     // X position
    write_oamdata(player_y);     // Y position
    write_oamdata(0);            // Tile 0
    write_oamdata(0x30);         // Priority 3, palette 0
}

// Handle D-pad input for player movement
static void handle_input() {
    wait_for_joypad();
    u8 btns = read_joy1h();

    if (btns & BTN_LEFT) {
        if (player_x > 0) --player_x;
    }
    if (btns & BTN_RIGHT) {
        if (player_x < 248) ++player_x;
    }
    if (btns & BTN_UP) {
        if (player_y > 0) --player_y;
    }
    if (btns & BTN_DOWN) {
        if (player_y < 208) ++player_y;
    }
}

// Update parallax scroll positions
// BG1 scrolls slower (far mountains)
// BG2 scrolls faster (near hills)
static void update_scroll() {
    // Auto-scroll both backgrounds (parallax effect)
    // BG1: 0.5 pixels per frame (far, slow)
    // BG2: 1.0 pixels per frame (near, fast)
    scroll_bg1 += 1;   // Half speed (using 8.8 fixed point)
    scroll_bg2 += 2;   // Full speed

    // Apply BG1 scroll (divide by 2 for sub-pixel)
    u16 bg1_pos = scroll_bg1 >> 1;
    set_bg1hofs_lo(bg1_pos & 0xFF);
    set_bg1hofs_hi(bg1_pos >> 8);
    set_bg1vofs_lo(0);
    set_bg1vofs_hi(0);

    // Apply BG2 scroll
    u16 bg2_pos = scroll_bg2 >> 1;
    set_bg2hofs_lo(bg2_pos & 0xFF);
    set_bg2hofs_hi(bg2_pos >> 8);
    set_bg2vofs_lo(0);
    set_bg2vofs_hi(0);
}

int main() {
    // Initialize player position (center of screen)
    player_x = 120;
    player_y = 180;

    // Initialize scroll
    scroll_bg1 = 0;
    scroll_bg2 = 0;
    frame = 0;

    // Background color is set by parallax_palette in crt0

    // Configure BG Mode 0 (4 layers, 2bpp each)
    // This gives us 4 colors per BG with good layering
    set_mode(bgmode::MODE_0);

    // Enable sprites and backgrounds (crt0 loads the tile data)
    set_tm(SCREEN_OBJ | SCREEN_BG1 | SCREEN_BG2);

    // Sprite size and base address set by crt0.s:
    // OBSEL = $03: base at VRAM $6000, size mode 0 (8x8/16x16)
    // Don't override - smiley face sprite is already loaded there

    // Enable joypad auto-read
    enable_joypad();

    // Turn screen on at full brightness
    screen_on(15);

    // Main game loop
    for (;;) {
        wait_vblank();
        update_oam();
        handle_input();
        update_scroll();
        ++frame;
    }

    return 0;
}
