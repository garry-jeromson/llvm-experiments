// INTEGRATION-TEST
// EXPECT: 4
int test_main(void) {
    int arr[6] = {10, 20, 30, 40, 50, 60};
    int *p1 = &arr[1];
    int *p2 = &arr[5];
    int diff = p2 - p1;
    return diff;
}
