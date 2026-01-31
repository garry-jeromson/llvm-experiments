/*
 * SNES Text Demo - Displays text and a hex counter
 * Compiled with LLVM W65816 backend
 *
 * Demonstrates:
 * - Mode 0 graphics with text rendering
 * - Font tile loading and tilemap manipulation
 * - Counter loop (using hex to avoid division bug)
 */

#include "snes.h"

/* ============================================================================
 * Helper Functions
 * ============================================================================ */

/* Write a byte to a hardware register address */
static void poke(unsigned int addr, unsigned char value) {
    *(volatile unsigned char *)addr = value;
}

/* Read a byte from a hardware register address */
static unsigned char peek(unsigned int addr) {
    return *(volatile unsigned char *)addr;
}

/* wait_vblank() is now provided by snes.h */

/* ============================================================================
 * Simplified Text Output
 * ============================================================================ */

/* Write a single character to the tilemap at (x, y)
 * Sets VRAM address and writes tile index
 */
static void put_char_at(unsigned int x, unsigned int y, unsigned char tile) {
    unsigned int vram_addr;

    /* Calculate VRAM address: tilemap at $1000, 32 tiles per row */
    vram_addr = 0x1000 + (y * 32) + x;

    /* Set VRAM address */
    poke(0x2115, 0x80);      /* VMAIN: Increment after high byte write */
    poke(0x2116, vram_addr & 0xFF);        /* VMADDL */
    poke(0x2117, (vram_addr >> 8) & 0xFF); /* VMADDH */

    /* Write tile index and attributes */
    poke(0x2118, tile);      /* VMDATAL: Tile number */
    poke(0x2119, 0x00);      /* VMDATAH: Attributes (palette 0, no flip) */
}

/* Write a fixed-length string at position (x, y)
 * Characters are converted: ASCII - 32 = tile index
 */
static void write_text(unsigned int x, unsigned int y,
                       unsigned char c0, unsigned char c1, unsigned char c2,
                       unsigned char c3, unsigned char c4) {
    if (c0) put_char_at(x + 0, y, c0 - 32);
    if (c1) put_char_at(x + 1, y, c1 - 32);
    if (c2) put_char_at(x + 2, y, c2 - 32);
    if (c3) put_char_at(x + 3, y, c3 - 32);
    if (c4) put_char_at(x + 4, y, c4 - 32);
}

/* Write a 4-digit hex number at position
 * Uses bit shifts instead of division (avoiding compiler bug)
 */
static void write_hex(unsigned int x, unsigned int y, unsigned int num) {
    unsigned char d0, d1, d2, d3;

    /* Extract hex digits using bit operations */
    d3 = (num >> 12) & 0x0F;
    d2 = (num >> 8) & 0x0F;
    d1 = (num >> 4) & 0x0F;
    d0 = num & 0x0F;

    /* Convert to tile index: 0-9 = tiles 16-25, A-F = tiles 33-38 */
    d3 = (d3 < 10) ? (16 + d3) : (33 + d3 - 10);
    d2 = (d2 < 10) ? (16 + d2) : (33 + d2 - 10);
    d1 = (d1 < 10) ? (16 + d1) : (33 + d1 - 10);
    d0 = (d0 < 10) ? (16 + d0) : (33 + d0 - 10);

    put_char_at(x + 0, y, d3);
    put_char_at(x + 1, y, d2);
    put_char_at(x + 2, y, d1);
    put_char_at(x + 3, y, d0);
}

/* Clear a section of the screen */
static void clear_area(unsigned int x, unsigned int y,
                       unsigned int w, unsigned int h) {
    unsigned int row, col;
    for (row = 0; row < h; row++) {
        for (col = 0; col < w; col++) {
            put_char_at(x + col, y + row, 0);  /* Space tile */
        }
    }
}

/* ============================================================================
 * Simple Delay - uses nested loops to avoid volatile codegen issues
 * ============================================================================ */

static void delay(unsigned int outer) {
    unsigned int i, j;
    for (i = 0; i < outer; i++) {
        for (j = 0; j < 100; j++) {
            /* Use inline asm NOP to prevent optimization */
            __asm__ volatile("nop");
        }
    }
}

/* ============================================================================
 * Main Program
 * ============================================================================ */

int main(void) {
    unsigned int counter;

    /* Clear the main screen area */
    clear_area(0, 0, 32, 28);

    /* Display static text using individual characters */
    /* "LLVM" at (6, 2) */
    put_char_at(6, 2, 'L' - 32);
    put_char_at(7, 2, 'L' - 32);
    put_char_at(8, 2, 'V' - 32);
    put_char_at(9, 2, 'M' - 32);

    /* "W65816" at (11, 2) */
    put_char_at(11, 2, 'W' - 32);
    put_char_at(12, 2, '6' - 32);
    put_char_at(13, 2, '5' - 32);
    put_char_at(14, 2, '8' - 32);
    put_char_at(15, 2, '1' - 32);
    put_char_at(16, 2, '6' - 32);

    /* "SNES" at (12, 4) */
    put_char_at(12, 4, 'S' - 32);
    put_char_at(13, 4, 'N' - 32);
    put_char_at(14, 4, 'E' - 32);
    put_char_at(15, 4, 'S' - 32);

    /* "DEMO" at (17, 4) */
    put_char_at(17, 4, 'D' - 32);
    put_char_at(18, 4, 'E' - 32);
    put_char_at(19, 4, 'M' - 32);
    put_char_at(20, 4, 'O' - 32);

    /* "COUNT:" at (10, 8) */
    put_char_at(10, 8, 'C' - 32);
    put_char_at(11, 8, 'O' - 32);
    put_char_at(12, 8, 'U' - 32);
    put_char_at(13, 8, 'N' - 32);
    put_char_at(14, 8, 'T' - 32);
    put_char_at(15, 8, ':' - 32);

    /* Turn screen on at full brightness */
    poke(0x2100, 0x0F);

    /* Main loop - increment and display counter (hex) */
    counter = 0;
    for (;;) {
        /* Wait for VBlank before writing to VRAM */
        wait_vblank();

        /* Display the counter value in hex */
        write_hex(17, 8, counter);

        /* Increment counter */
        counter = counter + 1;
    }

    return 0;  /* Never reached */
}
