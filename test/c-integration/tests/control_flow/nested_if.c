// INTEGRATION-TEST
// EXPECT: 3

int test_main(void) {
    int a = 10;
    int b = 20;
    int result;

    if (a > 5) {
        if (b > 15) {
            result = 3;
        } else {
            result = 2;
        }
    } else {
        result = 1;
    }
    return result;
}
