// INTEGRATION-TEST
// EXPECT: 15
int counter(void) {
    static int count = 0;
    count += 5;
    return count;
}

int test_main(void) {
    counter();
    counter();
    return counter();
}
