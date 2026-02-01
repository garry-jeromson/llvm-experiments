// INTEGRATION-TEST
// EXPECT: 65526

// Negation test - note: result is two's complement
// -10 in 16-bit signed = 0xFFF6 (65526 unsigned)
int test_main(void) {
    int a = 10;
    return -a;
}
