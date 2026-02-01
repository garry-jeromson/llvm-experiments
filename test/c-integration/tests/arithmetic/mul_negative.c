// INTEGRATION-TEST
// EXPECT: 65486

// Multiplication with negative result
// -5 * 10 = -50 (0xFFCE = 65486 unsigned)
int test_main(void) {
    int a = -5;
    int b = 10;
    return a * b;
}
