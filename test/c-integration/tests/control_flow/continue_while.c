// INTEGRATION-TEST
// EXPECT: 25
// Test continue in while loop - sum only odd values 1-9
int test_main(void) {
    int sum = 0;
    int i = 0;
    while (i < 10) {
        i++;
        if ((i & 1) == 0) continue;  // skip even i values
        sum += i;  // adds 1 + 3 + 5 + 7 + 9 = 25
    }
    return sum;
}
