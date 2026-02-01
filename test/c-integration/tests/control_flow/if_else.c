// INTEGRATION-TEST
// EXPECT: 20

int test_main(void) {
    int a = 10;
    int result;
    if (a > 5) {
        result = 20;
    } else {
        result = 30;
    }
    return result;
}
