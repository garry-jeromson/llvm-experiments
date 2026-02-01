// INTEGRATION-TEST
// EXPECT: 30

int test_main(void) {
    int a = 3;
    int result;
    if (a > 5) {
        result = 20;
    } else {
        result = 30;
    }
    return result;
}
