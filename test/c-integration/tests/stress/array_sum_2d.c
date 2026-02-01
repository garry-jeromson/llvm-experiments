// INTEGRATION-TEST
// EXPECT: 78

// Sum 2D array
int test_main(void) {
    int arr[3][4] = {
        {1, 2, 3, 4},
        {5, 6, 7, 8},
        {9, 10, 11, 12}
    };

    int sum = 0;
    for (int i = 0; i < 3; i++) {
        for (int j = 0; j < 4; j++) {
            sum += arr[i][j];
        }
    }
    return sum;  // 1+2+...+12 = 78
}
