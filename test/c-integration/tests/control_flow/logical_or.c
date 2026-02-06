// INTEGRATION-TEST
// EXPECT: 1
// Test logical OR with short-circuit evaluation

int side_effect_called = 0;

int side_effect(void) {
    side_effect_called = 1;
    return 0;
}

int test_main(void) {
    int a = 5;

    // Should short-circuit: side_effect not called because a is non-zero
    if (a || side_effect()) {
        // Good, entered the block
    } else {
        return 99;
    }

    if (side_effect_called) {
        return 88;  // side_effect was called when it shouldn't be
    }

    // Test OR with both false then true
    int x = 0;
    int y = 0;
    int z = 1;
    if (x || y || z) {
        return 1;
    }

    return 0;
}
