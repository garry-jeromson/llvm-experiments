// INTEGRATION-TEST
// EXPECT: 30
int test_main(void) {
    int result = 0;
    int x = 2;
    switch (x) {
        case 1:
            result = 10;
            break;
        case 2:
            result += 10;
            __attribute__((fallthrough));
        case 3:
            result += 20;
            break;
        default:
            result = 100;
    }
    return result;
}
