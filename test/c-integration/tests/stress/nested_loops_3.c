// INTEGRATION-TEST
// EXPECT: 120

// Triple nested loops
int test_main(void) {
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 5; j++) {
            for (int k = 0; k < 6; k++) {
                sum += 1;
            }
        }
    }
    return sum;  // 4 * 5 * 6 = 120
}
