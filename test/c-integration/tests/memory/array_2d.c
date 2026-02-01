// INTEGRATION-TEST
// EXPECT: 5

// 2D array access
int test_main(void) {
    int arr[2][3] = {{1, 2, 3}, {4, 5, 6}};
    return arr[1][1];  // 5
}
