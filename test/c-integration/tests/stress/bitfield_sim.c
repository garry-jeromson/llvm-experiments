// INTEGRATION-TEST
// EXPECT: 170

// Simulate bit field operations
int test_main(void) {
    unsigned int flags = 0;

    // Set bits
    flags |= (1 << 1);  // bit 1
    flags |= (1 << 3);  // bit 3
    flags |= (1 << 5);  // bit 5
    flags |= (1 << 7);  // bit 7

    // Check bits and accumulate
    int result = 0;
    if (flags & (1 << 1)) result += 10;
    if (flags & (1 << 3)) result += 20;
    if (flags & (1 << 5)) result += 40;
    if (flags & (1 << 7)) result += 100;

    return result;  // 10+20+40+100 = 170
}
