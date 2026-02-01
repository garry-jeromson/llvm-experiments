// INTEGRATION-TEST
// EXPECT: 65533

// Signed modulo with negative dividend
// -23 % 5 = -3 (0xFFFD = 65533 unsigned)
int test_main(void) {
    int a = -23;
    int b = 5;
    return a % b;
}
