/*
 * SNES SDK Example: Hello World (C)
 * Demonstrates basic SDK usage with the C API
 */

#include <snes.h>

/* Use static variables to avoid register pressure issues */
static snes_i16 player_x;
static snes_i16 player_y;

int main(void) {
    /* Initialize SDK */
    snes_init();

    /* Set background color (blue) */
    snes_set_bgcolor_rgb(0, 0, 15);

    /* Load sprite graphics to VRAM */
    snes_load_sprite_tiles();

    /* Set up sprite palette */
    snes_set_sprite_palette();

    /* Set up sprite base address and size mode */
    snes_sprites_set_obsel(0x0000, 0);  /* 8x8/16x16 sprites at VRAM 0 */

    /* Enable sprites on main screen */
    snes_set_main_screen(SNES_LAYER_OBJ);

    /* Turn on screen */
    snes_screen_on(15);

    /* Initialize player position */
    player_x = 128;
    player_y = 112;

    /* Main loop */
    for (;;) {
        /* Wait for vblank */
        snes_wait_vblank();

        /* Update joypad state */
        snes_joy_update();

        /* Move player with d-pad */
        if (snes_joy_held(0, SNES_BTN_LEFT))  player_x -= 2;
        if (snes_joy_held(0, SNES_BTN_RIGHT)) player_x += 2;
        if (snes_joy_held(0, SNES_BTN_UP))    player_y -= 2;
        if (snes_joy_held(0, SNES_BTN_DOWN))  player_y += 2;

        /* Clamp to screen bounds */
        if (player_x < 0) player_x = 0;
        if (player_x > 248) player_x = 248;
        if (player_y < 0) player_y = 0;
        if (player_y > 216) player_y = 216;

        /* Update sprite position and tile */
        snes_sprite_set_pos(0, player_x, (snes_u8)player_y);
        snes_sprite_set_tile(0, 0, 0, SNES_FALSE, SNES_FALSE);

        /* Upload sprites to OAM */
        snes_sprites_upload();
    }

    return 0;
}
