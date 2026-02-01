// INTEGRATION-TEST
// EXPECT: 10

int test_main(void) {
    int arr[4] = {1, 2, 3, 4};
    int sum = 0;
    for (int i = 0; i < 4; i++) {
        sum = sum + arr[i];
    }
    return sum;  // 1+2+3+4 = 10
}
