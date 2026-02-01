// INTEGRATION-TEST
// EXPECT: 1

// Signed comparison: -1 < 1 should be true
int test_main(void) {
    int a = -1;
    int b = 1;
    if (a < b) {
        return 1;
    }
    return 0;
}
