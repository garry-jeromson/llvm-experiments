// INTEGRATION-TEST
// EXPECT: 10
int test_main(void) {
    int a = 10;
    int b = a--;
    return b;
}
