// INTEGRATION-TEST
// EXPECT: 32768

// Overflow wraps around (32767 + 1 = -32768 signed = 32768 unsigned)
int test_main(void) {
    int a = 32767;
    a = a + 1;
    return a;
}
