// INTEGRATION-TEST
// EXPECT: 4

// Test volatile 8-bit comparison - matches SNES demo pattern
// The bug: if (menu_sel < 4) generates cmp #3; bne instead of cmp #4; bcc

static volatile unsigned char menu_sel;

static void menu_nav_down(void) {
    // This is the exact pattern from the demo that fails
    if (menu_sel < 4) menu_sel = menu_sel + 1;
}

unsigned int test_main(void) {
    // Start at 0, call menu_nav_down 5 times
    // Should increment: 0→1→2→3→4 then stop at 4
    menu_sel = 0;

    menu_nav_down();  // 0 < 4, so 0→1
    menu_nav_down();  // 1 < 4, so 1→2
    menu_nav_down();  // 2 < 4, so 2→3
    menu_nav_down();  // 3 < 4, so 3→4
    menu_nav_down();  // 4 is NOT < 4, so stays 4

    return menu_sel;  // Should be 4
}
