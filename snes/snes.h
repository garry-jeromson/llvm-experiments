/*
 * SNES PPU Register Definitions
 * For use with LLVM W65816 backend
 */

#ifndef SNES_H
#define SNES_H

/* PPU Registers */
#define INIDISP  (*(volatile unsigned char*)0x2100)  /* Display control */
#define CGADD    (*(volatile unsigned char*)0x2121)  /* CGRAM address */
#define CGDATA   (*(volatile unsigned char*)0x2122)  /* CGRAM data write */

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

#endif /* SNES_H */
