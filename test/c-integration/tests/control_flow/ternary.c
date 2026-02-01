// INTEGRATION-TEST
// EXPECT: 100

int test_main(void) {
    int a = 15;
    int result = (a > 10) ? 100 : 50;
    return result;
}
