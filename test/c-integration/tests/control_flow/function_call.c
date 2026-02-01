// INTEGRATION-TEST
// EXPECT: 25

int add(int a, int b) {
    return a + b;
}

int test_main(void) {
    return add(10, 15);
}
