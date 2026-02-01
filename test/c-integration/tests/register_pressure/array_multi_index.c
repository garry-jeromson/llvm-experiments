// INTEGRATION-TEST
// EXPECT: 100

// Multiple array accesses with different indices live
int test_main(void) {
    int arr[6] = {10, 20, 30, 40, 50, 60};
    int i = 0, j = 2, k = 5;
    // Three indices and array base all live
    return arr[i] + arr[j] + arr[k];
    // 10 + 30 + 60 = 100
}
