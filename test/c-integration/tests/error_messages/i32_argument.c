// INTEGRATION-TEST
// EXPECT-ERROR: 32-bit and 64-bit integer arguments are not supported on W65816

// Test that 32-bit function arguments produce a clear error message
short process_value(long x) {
    return (short)x;
}

short test_main(void) {
    return process_value(100000L);
}
