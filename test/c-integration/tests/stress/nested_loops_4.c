// INTEGRATION-TEST
// EXPECT: 24

// Four nested loops
int test_main(void) {
    int count = 0;
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            for (int k = 0; k < 2; k++) {
                for (int l = 0; l < 3; l++) {
                    count++;
                }
            }
        }
    }
    return count;  // 2*2*2*3 = 24
}
