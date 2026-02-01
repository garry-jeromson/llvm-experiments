// INTEGRATION-TEST
// EXPECT: 89

// Find max and min in array
int test_main(void) {
    int arr[5] = {23, 89, 12, 45, 67};
    int max = arr[0];
    int min = arr[0];

    for (int i = 1; i < 5; i++) {
        if (arr[i] > max) {
            max = arr[i];
        }
        if (arr[i] < min) {
            min = arr[i];
        }
    }
    // max = 89, min = 12
    return max;
}
