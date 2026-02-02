// INTEGRATION-TEST
// EXPECT: 2
// SKIP: XOR with stack-relative addressing bug causes incorrect SGE code when comparing against 0

// Test volatile 16-bit SIGNED greater-or-equal comparison
// Note: `val >= 0` gets transformed to `val > -1` which generates buggy XOR code

unsigned int test_main(void) {
    unsigned int result = 0;
    volatile int val;

    val = 0;
    if (val >= 0) result++;     // 0 >= 0 is true

    val = 1;
    if (val >= 0) result++;     // 1 >= 0 is true

    val = -1;
    if (val >= 0) result++;     // -1 >= 0 is false (BUG: returns true)

    val = 32767;
    if (val >= 32767) result++; // 32767 >= 32767 is true

    return result;  // Should be 3, but BUG causes 4
}
