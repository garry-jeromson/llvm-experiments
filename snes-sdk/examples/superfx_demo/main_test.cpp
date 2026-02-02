// Minimal test - just input and sprite movement
#include <snes/ppu.hpp>
#include <snes/input.hpp>

using namespace snes;
using namespace snes::ppu;
using namespace snes::input;

static volatile u8 sprite_x;
static volatile u8 prev_buttons;

int main() {
    sprite_x = 100;
    prev_buttons = 0;

    enable_joypad();
    screen_on(15);
    set_tm(SCREEN_BG1 | SCREEN_OBJ);

    for (;;) {
        wait_vblank();
        wait_for_joypad();

        u8 buttons = read_joy1l();

        // Move sprite on any button press
        if ((buttons & BTN_A) && !(prev_buttons & BTN_A)) {
            sprite_x = sprite_x + 10;
        }
        if ((buttons & BTN_B) && !(prev_buttons & BTN_B)) {
            sprite_x = sprite_x - 10;
        }

        prev_buttons = buttons;

        // Update sprite position
        set_oamaddr(0);
        write_oamdata(sprite_x);  // X
        write_oamdata(100);       // Y
        write_oamdata(0);         // Tile
        write_oamdata(0x30);      // Attr
    }

    return 0;
}
