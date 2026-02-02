// INTEGRATION-TEST
// EXPECT-ERROR: 32-bit and 64-bit return values are not supported on W65816

// Test that returning a 64-bit value produces a clear error message
long long test_main(void) {
    return 1000000000LL;
}
