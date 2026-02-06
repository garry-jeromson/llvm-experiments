/* Test sprite display - no input, fixed position */
#include <snes.h>

int main(void) {
    /* Initialize */
    snes_init();

    /* Blue background */
    snes_set_bgcolor_rgb(0, 0, 15);

    /* Load sprite graphics */
    snes_load_sprite_tiles();
    snes_set_sprite_palette();

    /* Configure sprites */
    snes_sprites_set_obsel(0x0000, 0);
    snes_set_main_screen(SNES_LAYER_OBJ);

    /* Set sprite 0 position and tile BEFORE screen on */
    snes_sprite_set_pos(0, 128, 112);
    snes_sprite_set_tile(0, 0, 0, SNES_FALSE, SNES_FALSE);
    snes_sprites_upload();

    /* Turn on screen */
    snes_screen_on(15);

    /* Simple loop - just wait */
    for (;;) {
        snes_wait_vblank();
    }

    return 0;
}
