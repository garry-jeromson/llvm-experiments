// INTEGRATION-TEST
// EXPECT: 50

// Deep expression tree requiring intermediate values
int test_main(void) {
    int a = 1;
    int b = 2;
    int c = 3;
    int d = 4;
    int sum1 = a + b;        // 3
    int sum2 = c + d;        // 7
    int total = sum1 + sum2; // 10
    int doubled = total + total; // 20
    // 1+2+3+4+3+7+10+20 = 50
    return a + b + c + d + sum1 + sum2 + total + doubled;
}
