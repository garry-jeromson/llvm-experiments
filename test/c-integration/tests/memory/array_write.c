// INTEGRATION-TEST
// EXPECT: 99

int test_main(void) {
    int arr[3] = {1, 2, 3};
    arr[1] = 99;
    return arr[1];
}
