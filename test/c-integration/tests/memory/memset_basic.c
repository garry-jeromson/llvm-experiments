// INTEGRATION-TEST
// EXPECT: 0

// Basic memset test - set array to zero
void *memset(void *s, int c, unsigned int n);

int test_main(void) {
    int arr[3] = {10, 20, 30};

    memset(arr, 0, 6);  // Set 6 bytes to 0

    return arr[0] + arr[1] + arr[2];  // 0 + 0 + 0 = 0
}
