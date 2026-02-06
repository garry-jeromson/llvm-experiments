// INTEGRATION-TEST
// EXPECT: 42
int test_main(void) {
    int a = 300;
    unsigned char b = (unsigned char)a;
    int c = (int)b;
    return c - 2;
}
