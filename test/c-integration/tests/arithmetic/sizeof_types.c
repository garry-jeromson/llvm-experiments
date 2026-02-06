// INTEGRATION-TEST
// EXPECT: 11
// Test sizeof operator

struct Point {
    int x;
    int y;
};

int test_main(void) {
    int arr[5];

    int s1 = sizeof(char);           // 1
    int s2 = sizeof(int);            // 2 (W65816 int is 16-bit)
    int s3 = sizeof(int *);          // 2 (16-bit pointers)
    int s4 = sizeof(arr);            // 10 (5 * 2)
    int s5 = sizeof(struct Point);   // 4 (2 + 2)

    // Verify expected sizes
    if (s1 != 1) return 100;
    if (s2 != 2) return 101;
    if (s3 != 2) return 102;
    if (s4 != 10) return 103;
    if (s5 != 4) return 104;

    // Return sum: 1 + 2 + 2 + 10 - 4 = 11
    return s1 + s2 + s3 + s4 - s5;
}
