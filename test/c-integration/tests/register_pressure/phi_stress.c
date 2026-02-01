// INTEGRATION-TEST
// EXPECT: 15

// PHI nodes with 3+ values - known to cause issues
int test_main(void) {
    int x = 1;
    int y = 2;
    int z = 3;
    if (x) {
        x = x + 4;  // 5
        y = y + 3;  // 5
        z = z + 2;  // 5
    }
    return x + y + z;  // 15
}
