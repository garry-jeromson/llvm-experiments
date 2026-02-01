/*
 * SNES SDK-Style Tech Demo
 * Demonstrates text output with counters
 * Simplified to avoid register pressure issues
 */

#include "snes.h"

/* Counter variables */
static unsigned int frame_counter = 0;
static unsigned int sprite_x = 128;
static unsigned int sprite_y = 112;

/* ============================================================================
 * Text Output Functions (simplified implementation)
 * ============================================================================ */

/* Write a single character to the tilemap at (x, y)
 * Sets VRAM address and writes tile index
 */
static void put_char_impl(unsigned int x, unsigned int y, unsigned char tile) {
    unsigned int vram_addr;

    /* Calculate VRAM address: tilemap at $1000, 32 tiles per row */
    vram_addr = 0x1000 + (y * 32) + x;

    /* Set VRAM address */
    VMAIN = 0x80;      /* Increment after high byte write */
    VMADDL = vram_addr & 0xFF;
    VMADDH = (vram_addr >> 8) & 0xFF;

    /* Write tile index and attributes */
    VMDATAL = tile;
    VMDATAH = 0x00;    /* Attributes (palette 0, no flip) */
}

/* Write an ASCII character at (x, y) */
void put_char(unsigned int x, unsigned int y, char c) {
    /* Font starts at ASCII 32 (space), tile 0 = space */
    put_char_impl(x, y, (unsigned char)(c - 32));
}

/* Write a 4-digit hex number at position (x, y) */
static void put_hex(unsigned int x, unsigned int y, unsigned int num) {
    unsigned char d0, d1, d2, d3;

    /* Extract hex digits using bit operations */
    d3 = (num >> 12) & 0x0F;
    d2 = (num >> 8) & 0x0F;
    d1 = (num >> 4) & 0x0F;
    d0 = num & 0x0F;

    /* Convert to ASCII: 0-9 = '0'-'9', A-F = 'A'-'F' */
    d3 = (d3 < 10) ? ('0' + d3) : ('A' + d3 - 10);
    d2 = (d2 < 10) ? ('0' + d2) : ('A' + d2 - 10);
    d1 = (d1 < 10) ? ('0' + d1) : ('A' + d1 - 10);
    d0 = (d0 < 10) ? ('0' + d0) : ('A' + d0 - 10);

    put_char(x + 0, y, d3);
    put_char(x + 1, y, d2);
    put_char(x + 2, y, d1);
    put_char(x + 3, y, d0);
}

/* ============================================================================
 * Main Program
 * ============================================================================ */

int main(void) {
    /* Display title - using individual put_char calls to avoid loop register pressure */
    /* Row 2: "SDK DEMO" */
    put_char(12, 2, 'S');
    put_char(13, 2, 'D');
    put_char(14, 2, 'K');
    put_char(16, 2, 'D');
    put_char(17, 2, 'E');
    put_char(18, 2, 'M');
    put_char(19, 2, 'O');

    /* Row 4: "W65816" */
    put_char(12, 4, 'W');
    put_char(13, 4, '6');
    put_char(14, 4, '5');
    put_char(15, 4, '8');
    put_char(16, 4, '1');
    put_char(17, 4, '6');

    /* Row 8: "FRAME:" */
    put_char(4, 8, 'F');
    put_char(5, 8, 'R');
    put_char(6, 8, 'A');
    put_char(7, 8, 'M');
    put_char(8, 8, 'E');
    put_char(9, 8, ':');

    /* Row 10: "X:" */
    put_char(4, 10, 'X');
    put_char(5, 10, ':');

    /* Row 12: "Y:" */
    put_char(4, 12, 'Y');
    put_char(5, 12, ':');

    /* Turn screen on */
    screen_on();

    /* Main loop */
    for (;;) {
        /* Wait for VBlank before writing to VRAM */
        wait_vblank();

        /* Update counters */
        sprite_x = (sprite_x + 1) & 0xFF;
        sprite_y = (sprite_y + 1) & 0xFF;

        /* Display hex values (avoids division) */
        put_hex(12, 8, frame_counter);
        put_hex(12, 10, sprite_x);
        put_hex(12, 12, sprite_y);

        /* Increment frame counter */
        frame_counter = frame_counter + 1;
    }

    return 0;  /* Never reached */
}
