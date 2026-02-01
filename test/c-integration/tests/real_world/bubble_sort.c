// INTEGRATION-TEST
// EXPECT: 1

// Bubble sort a small array, return first element (should be smallest)
int test_main(void) {
    int arr[5] = {5, 3, 1, 4, 2};

    // Bubble sort
    for (int i = 0; i < 4; i++) {
        for (int j = 0; j < 4 - i; j++) {
            if (arr[j] > arr[j + 1]) {
                int temp = arr[j];
                arr[j] = arr[j + 1];
                arr[j + 1] = temp;
            }
        }
    }

    return arr[0];  // Should be 1 (smallest)
}
