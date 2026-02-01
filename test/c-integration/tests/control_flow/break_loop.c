// INTEGRATION-TEST
// EXPECT: 6

// Break out of loop early
int test_main(void) {
    int sum = 0;
    for (int i = 0; i < 100; i++) {
        sum += i;
        if (sum >= 5) {
            break;
        }
    }
    // i=0: sum=0, i=1: sum=1, i=2: sum=3, i=3: sum=6 (>=5, break)
    return sum;
}
