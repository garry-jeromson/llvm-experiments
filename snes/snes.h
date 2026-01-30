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

/* PPU Registers - OAM (Sprites) */
#define OBSEL    (*(volatile unsigned char*)0x2101)  /* Object size and base address */
#define OAMADDL  (*(volatile unsigned char*)0x2102)  /* OAM address low */
#define OAMADDH  (*(volatile unsigned char*)0x2103)  /* OAM address high */
#define OAMDATA  (*(volatile unsigned char*)0x2104)  /* OAM data write */

/* PPU Registers - Screen Enable */
#define TM       (*(volatile unsigned char*)0x212C)  /* Main screen designation */

/* Hardware Status */
#define HVBJOY   (*(volatile unsigned char*)0x4212)  /* VBlank/HBlank/Joypad status */

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

/* ============================================================================
 * VBlank Synchronization
 * ============================================================================ */

/*
 * Wait for VBlank to start (safe time to update VRAM/OAM)
 * Waits until NOT in VBlank, then waits until VBlank starts
 */
static inline void wait_vblank(void) {
    /* Wait until we're NOT in VBlank (in case we're already in one) */
    while (HVBJOY & 0x80) {
        __asm__ volatile("nop");
    }
    /* Now wait until VBlank starts */
    while (!(HVBJOY & 0x80)) {
        __asm__ volatile("nop");
    }
}

/* ============================================================================
 * Sprite (OAM) Functions
 * ============================================================================ */

/* OAM constants */
#define OAM_SPRITE_COUNT  128
#define OAM_LOW_TABLE     0      /* Low table: 4 bytes per sprite */
#define OAM_HIGH_TABLE    512    /* High table: 32 bytes total */

/*
 * Set a sprite's position, tile, and attributes
 * index: Sprite number (0-127)
 * x: X position (0-255, bit 8 handled separately)
 * y: Y position (0-223 visible, 224-239 = offscreen)
 * tile: Tile number in sprite VRAM
 * attr: Attributes (palette, priority, flip)
 *       Bits 0-2: Palette (0-7)
 *       Bit 6: Horizontal flip
 *       Bit 7: Vertical flip
 *       Bits 4-5: Priority (0-3)
 */
static inline void set_sprite(unsigned int index, int x, int y,
                              unsigned char tile, unsigned char attr) {
    unsigned int addr;
    unsigned char hi_byte;
    unsigned int hi_addr;
    unsigned int bit_pos;

    if (index >= OAM_SPRITE_COUNT) return;

    /* Set low table address (4 bytes per sprite) */
    addr = index * 4;
    OAMADDL = addr & 0xFF;
    OAMADDH = (addr >> 8) & 0x01;

    /* Write 4 bytes: X low, Y, Tile, Attributes */
    OAMDATA = x & 0xFF;           /* X position (low 8 bits) */
    OAMDATA = y & 0xFF;           /* Y position */
    OAMDATA = tile;               /* Tile number */
    OAMDATA = attr;               /* Attributes */

    /* Update high table for X bit 8 and size */
    hi_addr = 512 + (index >> 2);  /* 4 sprites per byte */
    bit_pos = (index & 3) * 2;     /* 2 bits per sprite */

    /* Read current high table byte, modify, write back */
    /* Note: For simplicity, we just write the X high bit and size=0 (8x8) */
    OAMADDL = hi_addr & 0xFF;
    OAMADDH = (hi_addr >> 8) & 0x01;

    /* Compute the 2-bit value: bit 0 = X high, bit 1 = size */
    hi_byte = (x >> 8) & 0x01;  /* X bit 8 only, size = 0 (8x8) */

    /* For now, write just this sprite's bits (assuming we manage all sprites) */
    /* This is a simplification - in production code you'd read-modify-write */
    OAMDATA = hi_byte << bit_pos;
}

/*
 * Hide a sprite by moving it offscreen
 * index: Sprite number (0-127)
 */
static inline void hide_sprite(unsigned int index) {
    /* Y = 224 or higher moves sprite offscreen */
    set_sprite(index, 0, 240, 0, 0);
}

#endif /* SNES_H */
