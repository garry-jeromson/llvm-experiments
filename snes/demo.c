/*
 * SNES Demo - Sets screen to bright green
 * Compiled with LLVM W65816 backend
 *
 * This demonstrates C code compiled with the W65816 backend
 * running on an actual SNES emulator.
 */

/* Write a byte to a hardware register address */
static void poke(unsigned int addr, unsigned char value) {
    *(volatile unsigned char *)addr = value;
}

int main(void) {
    /* Set CGRAM address to 0 (backdrop color entry) */
    poke(0x2121, 0);

    /* Write BGR555 color for bright green
     * Green = 31, so color = (31 << 5) = 0x03E0
     * Write low byte first: 0xE0
     * Then high byte: 0x03
     */
    poke(0x2122, 0xE0);
    poke(0x2122, 0x03);

    /* Turn screen on: full brightness (0x0F) */
    poke(0x2100, 0x0F);

    return 0;
}
