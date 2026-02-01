// INTEGRATION-TEST
// EXPECT: 5

// Do-while loop (executes at least once)
int test_main(void) {
    int count = 0;
    int i = 0;
    do {
        count++;
        i++;
    } while (i < 5);
    return count;
}
