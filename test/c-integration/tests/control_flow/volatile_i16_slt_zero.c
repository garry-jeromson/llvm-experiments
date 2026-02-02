// INTEGRATION-TEST
// EXPECT: 10

// Test volatile 16-bit SIGNED less-than zero comparison
// This tests the optimized lshr by 15 pattern for val < 0
// Tests both negative (true) and positive (false) cases

unsigned int test_main(void) {
    volatile int val;

    // Test negative value: -100 < 0 is true
    val = -100;
    if (val < 0) {
        // Test positive value: 100 < 0 is false
        val = 100;
        if (val < 0) {
            return 0;  // Should NOT get here
        }
        return 10;     // Should get here
    }
    return 0;  // Should NOT get here
}
