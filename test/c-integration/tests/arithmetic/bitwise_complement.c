// INTEGRATION-TEST
// EXPECT: 42
int test_main(void) {
    int a = 0x00FF;
    int b = ~a;
    int c = (b >> 8) & 0xFF;
    return c - 213;
}
