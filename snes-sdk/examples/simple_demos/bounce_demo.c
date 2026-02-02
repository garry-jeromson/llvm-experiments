/*
 * SNES Bouncing Smiley Demo
 * Sprite bounces around the screen
 */

#include "snes.h"

/* Screen bounds (8x8 sprite) */
#define MAX_X 248
#define MAX_Y 216

int main(void) {
    /* Position (pixel coordinates) */
    unsigned int pos_x = 128;
    unsigned int pos_y = 112;

    /* Direction flags: 0 = negative, 1 = positive */
    unsigned int dir_x = 1;
    unsigned int dir_y = 1;

    /* Speed in pixels per frame */
    unsigned int speed = 2;

    /* Set a dark blue background */
    CGADD = 0;
    CGDATA = 0x00;
    CGDATA = 0x40;

    /* Turn screen on */
    INIDISP = 0x0F;

    /* Main loop */
    for (;;) {
        /* Wait for VBlank */
        wait_vblank();

        /* Update X position */
        if (dir_x) {
            pos_x = pos_x + speed;
            if (pos_x >= MAX_X) {
                pos_x = MAX_X;
                dir_x = 0;
            }
        } else {
            if (pos_x >= speed) {
                pos_x = pos_x - speed;
            } else {
                pos_x = 0;
                dir_x = 1;
            }
        }

        /* Update Y position */
        if (dir_y) {
            pos_y = pos_y + speed;
            if (pos_y >= MAX_Y) {
                pos_y = MAX_Y;
                dir_y = 0;
            }
        } else {
            if (pos_y >= speed) {
                pos_y = pos_y - speed;
            } else {
                pos_y = 0;
                dir_y = 1;
            }
        }

        /* Update sprite 0 position */
        OAMADDL = 0;
        OAMADDH = 0;
        OAMDATA = (unsigned char)pos_x;
        OAMDATA = (unsigned char)pos_y;
        OAMDATA = 0;
        OAMDATA = 0x30;
    }

    return 0;
}
