/* Minimal test - just set bgcolor and turn on screen */
#include <snes.h>

int main(void) {
    /* Set blue background directly via registers */
    *(volatile unsigned char*)0x2121 = 0;    /* CGADD = 0 */
    *(volatile unsigned char*)0x2122 = 0x00; /* Low byte */
    *(volatile unsigned char*)0x2122 = 0x7C; /* High byte (blue=31) */

    /* Turn on screen with full brightness */
    *(volatile unsigned char*)0x2100 = 0x0F;

    /* Infinite loop */
    for (;;) {
        /* Wait for vblank */
        while (!(*(volatile unsigned char*)0x4212 & 0x80)) {}
        while (*(volatile unsigned char*)0x4212 & 0x80) {}
    }

    return 0;
}
