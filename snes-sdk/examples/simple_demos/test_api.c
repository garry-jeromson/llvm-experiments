/* Test the API functions one by one */
#include <snes.h>

int main(void) {
    /* Just set bgcolor and turn on - minimal API test */
    snes_set_bgcolor_rgb(0, 0, 31);  /* Bright blue */
    snes_screen_on(15);              /* Full brightness */

    /* Infinite loop */
    for (;;) {
        snes_wait_vblank();
    }

    return 0;
}
