// INTEGRATION-TEST
// EXPECT: 80

int test_main(void) {
    int a = 10;
    int b = a << 3;   // 80
    return b;
}
