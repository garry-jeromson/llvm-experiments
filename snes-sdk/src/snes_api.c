/*
 * SNES SDK - C API Implementation
 * Direct hardware access for W65816 target
 */

#include <snes.h>

/* PPU Registers */
#define INIDISP  (*(volatile snes_u8*)0x2100)
#define OBSEL    (*(volatile snes_u8*)0x2101)
#define OAMADDL  (*(volatile snes_u8*)0x2102)
#define OAMADDH  (*(volatile snes_u8*)0x2103)
#define OAMDATA  (*(volatile snes_u8*)0x2104)
#define CGADD    (*(volatile snes_u8*)0x2121)
#define CGDATA   (*(volatile snes_u8*)0x2122)
#define TM       (*(volatile snes_u8*)0x212C)
#define HVBJOY   (*(volatile snes_u8*)0x4212)
#define JOY1L    (*(volatile snes_u8*)0x4218)
#define JOY1H    (*(volatile snes_u8*)0x4219)
#define NMITIMEN (*(volatile snes_u8*)0x4200)

/* OAM buffer in RAM */
static snes_u8 oam_low[512];   /* 128 sprites * 4 bytes */
static snes_u8 oam_high[32];   /* High bits for X position and size */

/* Joypad state */
static snes_u16 joy_current[2];
static snes_u16 joy_previous[2];

void snes_init(void) {
    snes_u16 i;

    /* Clear OAM buffers */
    for (i = 0; i < 512; i++) {
        oam_low[i] = 0;
    }
    for (i = 0; i < 32; i++) {
        oam_high[i] = 0;
    }

    /* Hide all sprites (Y = 224 = off-screen) */
    for (i = 0; i < 128; i++) {
        oam_low[i * 4 + 1] = 224;
    }

    /* Clear joypad state */
    joy_current[0] = 0;
    joy_current[1] = 0;
    joy_previous[0] = 0;
    joy_previous[1] = 0;

    /* Enable joypad auto-read */
    NMITIMEN = 0x01;
}

void snes_screen_off(void) {
    INIDISP = 0x80;  /* Force blank */
}

void snes_screen_on(snes_u8 brightness) {
    if (brightness > 15) brightness = 15;
    INIDISP = brightness;
}

void snes_set_bgcolor_rgb(snes_u8 r, snes_u8 g, snes_u8 b) {
    snes_u16 color;
    if (r > 31) r = 31;
    if (g > 31) g = 31;
    if (b > 31) b = 31;
    color = (snes_u16)r | ((snes_u16)g << 5) | ((snes_u16)b << 10);
    CGADD = 0;
    CGDATA = (snes_u8)(color & 0xFF);
    CGDATA = (snes_u8)(color >> 8);
}

void snes_sprites_set_obsel(snes_u16 base_addr, snes_u8 size_mode) {
    /* base_addr is word address (divide by 8K to get value) */
    snes_u8 val = (snes_u8)((base_addr >> 13) & 0x07);
    val |= (size_mode & 0x07) << 5;
    OBSEL = val;
}

void snes_set_main_screen(snes_u8 layer_mask) {
    TM = layer_mask;
}

void snes_wait_vblank(void) {
    /* Wait for vblank flag to be set */
    while (!(HVBJOY & 0x80)) {
        /* spin */
    }
    /* Wait for vblank to end (ensures we catch the start of next vblank) */
    while (HVBJOY & 0x80) {
        /* spin */
    }
}

void snes_joy_update(void) {
    /* Wait for auto-read to complete */
    while (HVBJOY & 0x01) {
        /* spin */
    }

    /* Save previous state */
    joy_previous[0] = joy_current[0];
    joy_previous[1] = joy_current[1];

    /* Read new state */
    joy_current[0] = (snes_u16)JOY1L | ((snes_u16)JOY1H << 8);
}

snes_bool snes_joy_held(snes_u8 joypad_id, snes_u16 button_mask) {
    if (joypad_id > 1) return SNES_FALSE;
    return (joy_current[joypad_id] & button_mask) ? SNES_TRUE : SNES_FALSE;
}

snes_bool snes_joy_pressed(snes_u8 joypad_id, snes_u16 button_mask) {
    snes_u16 curr, prev;
    if (joypad_id > 1) return SNES_FALSE;
    curr = joy_current[joypad_id];
    prev = joy_previous[joypad_id];
    return ((curr & button_mask) && !(prev & button_mask)) ? SNES_TRUE : SNES_FALSE;
}

snes_i16 snes_clamp(snes_i16 val, snes_i16 lo, snes_i16 hi) {
    if (val < lo) return lo;
    if (val > hi) return hi;
    return val;
}

void snes_sprite_set_pos(snes_u8 id, snes_i16 x, snes_u8 y) {
    snes_u16 idx;
    snes_u8 byte_idx, bit_pos;

    if (id >= 128) return;

    idx = (snes_u16)id * 4;
    oam_low[idx + 0] = (snes_u8)(x & 0xFF);
    oam_low[idx + 1] = y;

    /* Set X high bit in high table */
    byte_idx = id >> 2;
    bit_pos = (id & 0x03) << 1;
    if (x & 0x100) {
        oam_high[byte_idx] |= (snes_u8)(1 << bit_pos);
    } else {
        oam_high[byte_idx] &= (snes_u8)~(1 << bit_pos);
    }
}

void snes_sprite_set_tile(snes_u8 id, snes_u16 tile, snes_u8 palette, snes_bool hflip, snes_bool vflip) {
    snes_u16 idx;
    snes_u8 attr;

    if (id >= 128) return;

    idx = (snes_u16)id * 4;
    oam_low[idx + 2] = (snes_u8)(tile & 0xFF);

    attr = (snes_u8)((tile >> 8) & 0x01);  /* Name table select */
    attr |= (palette & 0x07) << 1;          /* Palette */
    /* Priority defaults to 0 */
    if (hflip) attr |= 0x40;
    if (vflip) attr |= 0x80;

    oam_low[idx + 3] = attr;
}

void snes_sprite_hide(snes_u8 id) {
    if (id >= 128) return;
    /* Move sprite off-screen */
    oam_low[(snes_u16)id * 4 + 1] = 224;
}

void snes_sprites_upload(void) {
    snes_u16 i;

    /* Set OAM address to 0 */
    OAMADDL = 0;
    OAMADDH = 0;

    /* Upload low table (512 bytes) */
    for (i = 0; i < 512; i++) {
        OAMDATA = oam_low[i];
    }

    /* Upload high table (32 bytes) */
    for (i = 0; i < 32; i++) {
        OAMDATA = oam_high[i];
    }
}
