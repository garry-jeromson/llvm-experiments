// INTEGRATION-TEST
// EXPECT: 1
int test_main(void) {
    int arr[4] = {1, 2, 3, 4};
    int *p1 = &arr[0];
    int *p2 = &arr[2];
    int result = 0;
    if (p1 < p2) result = 1;
    return result;
}
