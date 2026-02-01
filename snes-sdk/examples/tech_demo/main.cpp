// SNES SDK Tech Demo
//
// Controls:
//   D-pad Up/Down: Navigate menu
//   A: Select demo
//   B: Return to menu

#include <snes/ppu.hpp>
#include <snes/input.hpp>

using namespace snes;
using namespace snes::ppu;
using namespace snes::input;

// ============================================================
// Constants
// ============================================================

// Demo states
enum DemoState : u8 {
    STATE_MENU     = 0,
    STATE_PARALLAX = 1,
    STATE_MOSAIC   = 2,
    STATE_INPUT    = 3,
    STATE_PALETTE  = 4,
    STATE_SPRITE   = 5
};

// Menu configuration
static constexpr u8 MENU_ITEM_COUNT = 5;
static constexpr u8 MENU_REPEAT_DELAY = 12;  // Frames between key repeats

// Screen boundaries
static constexpr u8 SCREEN_WIDTH = 256;
static constexpr u8 SCREEN_HEIGHT = 224;
static constexpr u8 SPRITE_MAX_X = SCREEN_WIDTH - 8;   // 248
static constexpr u8 SPRITE_MAX_Y = SCREEN_HEIGHT - 16; // 208

// Sprite default positions
static constexpr u8 SPRITE_CENTER_X = 128;
static constexpr u8 SPRITE_CENTER_Y = 112;

// OAM attribute for sprites (palette 3, priority 0)
static constexpr u8 SPRITE_ATTR = 0x30;

// Global state
static volatile u8 g_state;
static volatile u8 menu_sel;
static volatile u16 scroll_x;
static volatile u8 sprite_x;
static volatile u8 sprite_y;
static volatile u8 mosaic_lvl;
static volatile u8 pal_phase;

// ============================================================
// Menu
// ============================================================

// Menu delay counter for input repeat
static volatile u8 menu_delay;

static u8 menu_check_delay() {
    u8 d = menu_delay;
    if (d != 0) {
        menu_delay = d - 1;
        return 1;
    }
    return 0;
}

static void menu_nav_up() {
    if (menu_sel != 0) menu_sel = menu_sel - 1;
    menu_delay = MENU_REPEAT_DELAY;
}

static void menu_nav_down() {
    if (menu_sel < MENU_ITEM_COUNT - 1) menu_sel = menu_sel + 1;
    menu_delay = MENU_REPEAT_DELAY;
}

static void menu_update() {
    wait_for_joypad();
    if (menu_check_delay()) return;

    u8 hi = read_joy1h();
    u8 lo = read_joy1l();

    if (hi & BTN_UP) menu_nav_up();
    if (hi & BTN_DOWN) menu_nav_down();
    if (lo & BTN_A) g_state = menu_sel + 1;
}

static void menu_draw() {
    static constexpr u8 CURSOR_X = 16;
    static constexpr u8 CURSOR_BASE_Y = 40;
    static constexpr u8 MENU_ITEM_HEIGHT = 16;

    set_tm(SCREEN_BG1 | SCREEN_BG3 | SCREEN_OBJ);
    set_bg1hofs_lo(0);
    set_bg1hofs_hi(0);
    set_oamaddr(0);
    write_oamdata(CURSOR_X);
    write_oamdata(CURSOR_BASE_Y + menu_sel * MENU_ITEM_HEIGHT);
    write_oamdata(0);  // Tile 0
    write_oamdata(SPRITE_ATTR);
}

// ============================================================
// Parallax
// ============================================================

static void parallax_update() {
    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & BTN_B) { g_state = STATE_MENU; return; }
    if (hi & BTN_LEFT)  { if (sprite_x != 0) sprite_x = sprite_x - 1; }
    if (hi & BTN_RIGHT) { if (sprite_x < SPRITE_MAX_X) sprite_x = sprite_x + 1; }
    if (hi & BTN_UP)    { if (sprite_y != 0) sprite_y = sprite_y - 1; }
    if (hi & BTN_DOWN)  { if (sprite_y < SPRITE_MAX_Y) sprite_y = sprite_y + 1; }
    scroll_x = scroll_x + 1;
}

static void parallax_draw() {
    set_tm(SCREEN_BG1 | SCREEN_BG2 | SCREEN_OBJ);
    u16 sx = scroll_x;
    // BG1 scrolls at half speed (parallax effect)
    set_bg1hofs_lo((sx >> 1) & 0xFF);
    set_bg1hofs_hi((sx >> 9) & 0xFF);
    // BG2 scrolls at full speed
    set_bg2hofs_lo(sx & 0xFF);
    set_bg2hofs_hi((sx >> 8) & 0xFF);
    set_oamaddr(0);
    write_oamdata(sprite_x);
    write_oamdata(sprite_y);
    write_oamdata(0);  // Tile 0
    write_oamdata(SPRITE_ATTR);
}

// ============================================================
// Mosaic - No frame counting, cycle every other frame approx
// ============================================================

static void mosaic_update() {
    static constexpr u8 MOSAIC_MAX = 0x0F;
    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & BTN_B) { g_state = STATE_MENU; return; }
    // Cycle mosaic level 0-15
    mosaic_lvl = (mosaic_lvl + 1) & MOSAIC_MAX;
}

static void mosaic_draw() {
    static constexpr u8 MOSAIC_BG_MASK = 0x03;  // Apply to BG1 and BG2
    static constexpr u8 OFFSCREEN_Y = 240;       // Move sprite off screen

    set_tm(SCREEN_BG1 | SCREEN_BG2);
    set_mosaic(mosaic_lvl, MOSAIC_BG_MASK);
    // Hide sprite by moving off screen
    set_oamaddr(0);
    write_oamdata(0);
    write_oamdata(OFFSCREEN_Y);
    write_oamdata(0);
    write_oamdata(0);
}

// ============================================================
// Input Test
// ============================================================

static void input_update() {
    // D-pad movement offsets from center
    static constexpr u8 MOVE_OFFSET = 32;

    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & BTN_B) { g_state = STATE_MENU; return; }

    // Start at center, move based on D-pad
    sprite_x = SPRITE_CENTER_X;
    sprite_y = SPRITE_CENTER_Y;
    if (hi & BTN_UP)    sprite_y = SPRITE_CENTER_Y - MOVE_OFFSET;
    if (hi & BTN_DOWN)  sprite_y = SPRITE_CENTER_Y + MOVE_OFFSET;
    if (hi & BTN_LEFT)  sprite_x = SPRITE_CENTER_X - MOVE_OFFSET;
    if (hi & BTN_RIGHT) sprite_x = SPRITE_CENTER_X + MOVE_OFFSET;
}

static void input_draw() {
    set_tm(SCREEN_BG1 | SCREEN_OBJ);
    set_oamaddr(0);
    write_oamdata(sprite_x);
    write_oamdata(sprite_y);
    write_oamdata(0);  // Tile 0
    write_oamdata(SPRITE_ATTR);
}

// ============================================================
// Palette Animation
// ============================================================

static void palette_update() {
    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & BTN_B) { g_state = STATE_MENU; return; }
    pal_phase = pal_phase + 1;
}

static void palette_draw() {
    static constexpr u8 COLOR_MAX = 31;   // Max color component value (5-bit)
    static constexpr u8 OFFSCREEN_Y = 240;

    set_tm(SCREEN_BG1 | SCREEN_BG2);
    // Animate palette entry 1 (cycle red component)
    set_cgadd(1);
    set_cgdata((pal_phase >> 2) & COLOR_MAX);
    set_cgdata(0);
    // Hide sprite
    set_oamaddr(0);
    write_oamdata(0);
    write_oamdata(OFFSCREEN_Y);
    write_oamdata(0);
    write_oamdata(0);
}

// ============================================================
// Sprite Animation
// ============================================================

static void sprite_update() {
    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & BTN_B) { g_state = STATE_MENU; return; }
    if (hi & BTN_LEFT)  { if (sprite_x != 0) sprite_x = sprite_x - 1; }
    if (hi & BTN_RIGHT) { if (sprite_x < SPRITE_MAX_X) sprite_x = sprite_x + 1; }
    if (hi & BTN_UP)    { if (sprite_y != 0) sprite_y = sprite_y - 1; }
    if (hi & BTN_DOWN)  { if (sprite_y < SPRITE_MAX_Y) sprite_y = sprite_y + 1; }
    pal_phase = pal_phase + 1;  // Reuse as animation frame counter
}

static void sprite_draw() {
    static constexpr u8 ANIM_FRAME_MASK = 3;   // 4 animation frames
    static constexpr u8 ANIM_SPEED_SHIFT = 3;  // Slow down animation

    set_tm(SCREEN_BG1 | SCREEN_OBJ);
    set_oamaddr(0);
    write_oamdata(sprite_x);
    write_oamdata(sprite_y);
    write_oamdata((pal_phase >> ANIM_SPEED_SHIFT) & ANIM_FRAME_MASK);  // Animated tile
    write_oamdata(SPRITE_ATTR);
}

// ============================================================
// Main
// ============================================================

int main() {
    static constexpr u8 FULL_BRIGHTNESS = 15;
    static constexpr u8 SPRITE_START_X = 120;
    static constexpr u8 SPRITE_START_Y = 100;

    // Initialize state
    g_state = STATE_MENU;
    menu_sel = 0;
    scroll_x = 0;
    sprite_x = SPRITE_START_X;
    sprite_y = SPRITE_START_Y;
    mosaic_lvl = 0;
    pal_phase = 0;
    menu_delay = 0;

    enable_joypad();
    screen_on(FULL_BRIGHTNESS);

    // Main loop
    for (;;) {
        wait_vblank();

        switch (g_state) {
            case STATE_MENU:     menu_update();     menu_draw();     break;
            case STATE_PARALLAX: parallax_update(); parallax_draw(); break;
            case STATE_MOSAIC:   mosaic_update();   mosaic_draw();   break;
            case STATE_INPUT:    input_update();    input_draw();    break;
            case STATE_PALETTE:  palette_update();  palette_draw();  break;
            case STATE_SPRITE:   sprite_update();   sprite_draw();   break;
        }
    }

    return 0;
}
