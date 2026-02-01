// INTEGRATION-TEST
// EXPECT: 45

int test_main(void) {
    int arr[10] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    int sum = 0;
    for (int i = 0; i < 10; i++) {
        sum = sum + arr[i];
    }
    return sum;  // 0+1+2+...+9 = 45
}
