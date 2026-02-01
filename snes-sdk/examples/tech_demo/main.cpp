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
    menu_delay = 12;
}

static void menu_nav_down() {
    if (menu_sel != 4) menu_sel = menu_sel + 1;
    menu_delay = 12;
}

static void menu_update() {
    wait_for_joypad();
    if (menu_check_delay()) return;

    u8 hi = read_joy1h();
    u8 lo = read_joy1l();

    if (hi & 0x08) menu_nav_up();
    if (hi & 0x04) menu_nav_down();
    if (lo & 0x80) g_state = menu_sel + 1;
}

static void menu_draw() {
    set_tm(SCREEN_BG1 | SCREEN_BG3 | SCREEN_OBJ);
    set_bg1hofs_lo(0);
    set_bg1hofs_hi(0);
    set_oamaddr(0);
    write_oamdata(16);
    write_oamdata(40 + menu_sel * 16);
    write_oamdata(0);
    write_oamdata(0x30);
}

// ============================================================
// Parallax
// ============================================================

static void parallax_update() {
    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & 0x80) { g_state = 0; return; }
    if (hi & 0x02) { if (sprite_x != 0) sprite_x = sprite_x - 1; }
    if (hi & 0x01) { if (sprite_x != 248) sprite_x = sprite_x + 1; }
    if (hi & 0x08) { if (sprite_y != 0) sprite_y = sprite_y - 1; }
    if (hi & 0x04) { if (sprite_y != 208) sprite_y = sprite_y + 1; }
    scroll_x = scroll_x + 1;
}

static void parallax_draw() {
    set_tm(SCREEN_BG1 | SCREEN_BG2 | SCREEN_OBJ);
    u16 sx = scroll_x;
    set_bg1hofs_lo((sx >> 1) & 0xFF);
    set_bg1hofs_hi((sx >> 9) & 0xFF);
    set_bg2hofs_lo(sx & 0xFF);
    set_bg2hofs_hi((sx >> 8) & 0xFF);
    set_oamaddr(0);
    write_oamdata(sprite_x);
    write_oamdata(sprite_y);
    write_oamdata(0);
    write_oamdata(0x30);
}

// ============================================================
// Mosaic - No frame counting, cycle every other frame approx
// ============================================================

static void mosaic_update() {
    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & 0x80) { g_state = 0; return; }
    // Simple increment, no frame counting
    mosaic_lvl = (mosaic_lvl + 1) & 0x0F;
}

static void mosaic_draw() {
    set_tm(SCREEN_BG1 | SCREEN_BG2);
    set_mosaic(mosaic_lvl, 0x03);
    set_oamaddr(0);
    write_oamdata(0);
    write_oamdata(240);
    write_oamdata(0);
    write_oamdata(0);
}

// ============================================================
// Input Test
// ============================================================

static void input_update() {
    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & 0x80) { g_state = 0; return; }
    sprite_x = 128;
    sprite_y = 112;
    if (hi & 0x08) sprite_y = 80;
    if (hi & 0x04) sprite_y = 144;
    if (hi & 0x02) sprite_x = 96;
    if (hi & 0x01) sprite_x = 160;
}

static void input_draw() {
    set_tm(SCREEN_BG1 | SCREEN_OBJ);
    set_oamaddr(0);
    write_oamdata(sprite_x);
    write_oamdata(sprite_y);
    write_oamdata(0);
    write_oamdata(0x30);
}

// ============================================================
// Palette
// ============================================================

static void palette_update() {
    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & 0x80) { g_state = 0; return; }
    pal_phase = pal_phase + 1;
}

static void palette_draw() {
    set_tm(SCREEN_BG1 | SCREEN_BG2);
    set_cgadd(1);
    set_cgdata((pal_phase >> 2) & 31);
    set_cgdata(0);
    set_oamaddr(0);
    write_oamdata(0);
    write_oamdata(240);
    write_oamdata(0);
    write_oamdata(0);
}

// ============================================================
// Sprite Animation
// ============================================================

static void sprite_update() {
    wait_for_joypad();
    u8 hi = read_joy1h();
    if (hi & 0x80) { g_state = 0; return; }
    if (hi & 0x02) { if (sprite_x != 0) sprite_x = sprite_x - 1; }
    if (hi & 0x01) { if (sprite_x != 240) sprite_x = sprite_x + 1; }
    if (hi & 0x08) { if (sprite_y != 0) sprite_y = sprite_y - 1; }
    if (hi & 0x04) { if (sprite_y != 208) sprite_y = sprite_y + 1; }
    pal_phase = pal_phase + 1;  // reuse pal_phase as anim counter
}

static void sprite_draw() {
    set_tm(SCREEN_BG1 | SCREEN_OBJ);
    set_oamaddr(0);
    write_oamdata(sprite_x);
    write_oamdata(sprite_y);
    write_oamdata((pal_phase >> 3) & 3);
    write_oamdata(0x30);
}

// ============================================================
// Main
// ============================================================

int main() {
    g_state = 0;
    menu_sel = 0;
    scroll_x = 0;
    sprite_x = 120;
    sprite_y = 100;
    mosaic_lvl = 0;
    pal_phase = 0;
    menu_delay = 0;

    enable_joypad();
    screen_on(15);

    for (;;) {
        wait_vblank();

        if (g_state == 0) { menu_update(); menu_draw(); }
        if (g_state == 1) { parallax_update(); parallax_draw(); }
        if (g_state == 2) { mosaic_update(); mosaic_draw(); }
        if (g_state == 3) { input_update(); input_draw(); }
        if (g_state == 4) { palette_update(); palette_draw(); }
        if (g_state == 5) { sprite_update(); sprite_draw(); }
    }

    return 0;
}
