// INTEGRATION-TEST
// EXPECT: 42

int test_main(void) {
    int val = 10;
    int *ptr = &val;
    *ptr = 42;
    return val;
}
