// INTEGRATION-TEST
// EXPECT: 5

int test_main(void) {
    int a = 80;
    int b = a >> 4;   // 5
    return b;
}
