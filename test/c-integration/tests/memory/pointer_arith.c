// INTEGRATION-TEST
// EXPECT: 30

int test_main(void) {
    int arr[3] = {10, 20, 30};
    int *ptr = arr;
    ptr = ptr + 2;  // Point to arr[2]
    return *ptr;    // 30
}
