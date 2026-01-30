/*
 * SNES PPU Register Definitions
 * For use with LLVM W65816 backend
 */

#ifndef SNES_H
#define SNES_H

/* PPU Registers - Display */
#define INIDISP  (*(volatile unsigned char*)0x2100)  /* Display control */

/* PPU Registers - VRAM */
#define VMAIN    (*(volatile unsigned char*)0x2115)  /* VRAM address increment mode */
#define VMADDL   (*(volatile unsigned char*)0x2116)  /* VRAM address low */
#define VMADDH   (*(volatile unsigned char*)0x2117)  /* VRAM address high */
#define VMDATAL  (*(volatile unsigned char*)0x2118)  /* VRAM data write low */
#define VMDATAH  (*(volatile unsigned char*)0x2119)  /* VRAM data write high */

/* PPU Registers - Palette */
#define CGADD    (*(volatile unsigned char*)0x2121)  /* CGRAM address */
#define CGDATA   (*(volatile unsigned char*)0x2122)  /* CGRAM data write */

/* Screen dimensions (Mode 0, 32x32 tilemap) */
#define SCREEN_WIDTH  32
#define SCREEN_HEIGHT 28   /* Visible area */

/* Tilemap configuration (set by crt0) */
#define TILEMAP_VRAM_ADDR  0x1000  /* BG1 tilemap at VRAM $1000 */
#define FONT_FIRST_CHAR    32      /* First ASCII char in font (space) */

/* Set backdrop (background) color
 * r, g, b: 0-31 each (5-bit values)
 * SNES uses BGR555 format: bbbbbggg_ggrrrrrr
 */
static inline void set_bgcolor(unsigned char r, unsigned char g, unsigned char b) {
    unsigned int color = ((unsigned int)b << 10) | ((unsigned int)g << 5) | r;
    CGADD = 0;              /* Select palette entry 0 (backdrop) */
    CGDATA = color & 0xFF;  /* Write low byte */
    CGDATA = color >> 8;    /* Write high byte */
}

/* Turn screen on at full brightness */
static inline void screen_on(void) {
    INIDISP = 0x0F;  /* Brightness = 15 (max), force blank off */
}

/* Turn screen off (force blank) */
static inline void screen_off(void) {
    INIDISP = 0x80;  /* Force blank on */
}

/* ============================================================================
 * Text Output Functions
 * ============================================================================ */

/*
 * Write a single character to the tilemap at (x, y)
 * x: column (0-31)
 * y: row (0-27)
 * c: ASCII character (32-95 supported)
 */
void put_char(unsigned int x, unsigned int y, char c);

/*
 * Write a null-terminated string starting at (x, y)
 * Wraps to next line if x exceeds screen width
 */
void put_text(unsigned int x, unsigned int y, const char *str);

/*
 * Write a 16-bit unsigned number as decimal at (x, y)
 * Right-aligned, with leading spaces removed
 */
void put_number(unsigned int x, unsigned int y, unsigned int num);

/*
 * Clear the entire screen (fill tilemap with space characters)
 */
void clear_screen(void);

#endif /* SNES_H */
