// INTEGRATION-TEST
// EXPECT-ERROR: 32-bit and 64-bit return values are not supported on W65816

// Test that returning a 32-bit value produces a clear error message
long test_main(void) {
    return 100000L;
}
