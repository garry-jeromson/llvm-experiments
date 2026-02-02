/*
 * SNES Demo - Simple version using direct memory access
 * Compiled with LLVM W65816 backend
 */

/* Global volatile pointers avoid complex stack-relative addressing */
volatile unsigned char * const INIDISP = (volatile unsigned char *)0x2100;
volatile unsigned char * const CGADD = (volatile unsigned char *)0x2121;
volatile unsigned char * const CGDATA = (volatile unsigned char *)0x2122;

int main(void) {
    /* Set CGRAM address to 0 (backdrop color) */
    *CGADD = 0;

    /* Write BGR555 color: bright green = 0x03E0 (R=0, G=31, B=0) */
    /* Low byte first: 0xE0 */
    *CGDATA = 0xE0;
    /* High byte: 0x03 */
    *CGDATA = 0x03;

    /* Turn screen on: brightness = 15 */
    *INIDISP = 0x0F;

    return 0;
}
