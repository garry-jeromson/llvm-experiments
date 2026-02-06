// INTEGRATION-TEST
// EXPECT: 30
int test_main(void) {
    int a = 10;
    int b = (a += 5, a + 15);
    return b;
}
