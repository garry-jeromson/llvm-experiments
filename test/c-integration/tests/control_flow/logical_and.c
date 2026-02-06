// INTEGRATION-TEST
// EXPECT: 1
// Test logical AND with short-circuit evaluation

int side_effect_called = 0;

int side_effect(void) {
    side_effect_called = 1;
    return 1;
}

int test_main(void) {
    int a = 0;
    int b = 5;

    // Should short-circuit: side_effect not called because a is 0
    if (a && side_effect()) {
        return 99;
    }

    if (side_effect_called) {
        return 88;  // side_effect was called when it shouldn't be
    }

    // Should evaluate both: b is non-zero
    if (b && (b > 3)) {
        return 1;
    }

    return 0;
}
