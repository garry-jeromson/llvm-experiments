// INTEGRATION-TEST
// EXPECT: 0

// Unsigned comparison: 0xFFFF > 1 should be true
// But as signed: -1 < 1, so this tests unsigned specifically
unsigned int test_main(void) {
    unsigned int a = 65535;  // 0xFFFF
    unsigned int b = 1;
    if (a < b) {
        return 1;  // Would be wrong
    }
    return 0;  // Correct: 65535 is not less than 1
}
