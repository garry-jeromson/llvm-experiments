// INTEGRATION-TEST
// EXPECT: 42
int test_main(void) {
    int a = 128;
    a >>= 2;
    a ^= 10;
    return a;
}
