// INTEGRATION-TEST
// EXPECT: 7

int test_main(void) {
    int a = 0x0F;
    int b = 0xF3;
    int c = a & b;    // 0x03
    int d = a | b;    // 0xFF
    int e = a ^ b;    // 0xFC
    // c=3, d=255, e=252; We need a small result
    return c + 4;     // 7
}
