// INTEGRATION-TEST
// EXPECT: 36

// 2x2 matrix operations
int test_main(void) {
    int a[2][2] = {{1, 2}, {3, 4}};
    int b[2][2] = {{5, 6}, {7, 8}};
    int c[2][2];

    // Matrix addition
    for (int i = 0; i < 2; i++) {
        for (int j = 0; j < 2; j++) {
            c[i][j] = a[i][j] + b[i][j];
        }
    }

    // Sum all elements of result
    return c[0][0] + c[0][1] + c[1][0] + c[1][1];  // 6+8+10+12 = 36... wait
    // a+b: {{6,8},{10,12}} -> 6+8+10+12=36
}
