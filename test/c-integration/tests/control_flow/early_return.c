// INTEGRATION-TEST
// EXPECT: 42

// Early return from function
int test_main(void) {
    int x = 10;
    if (x > 5) {
        return 42;
    }
    return 0;  // Should not reach here
}
