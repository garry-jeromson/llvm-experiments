// INTEGRATION-TEST
// EXPECT: 65531

// Signed division with negative result
// -25 / 5 = -5 (0xFFFB = 65531 unsigned)
int test_main(void) {
    int a = -25;
    int b = 5;
    return a / b;
}
