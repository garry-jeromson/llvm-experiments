// INTEGRATION-TEST
// EXPECT: 180

// Function call while keeping multiple variables live
int double_it(int x) { return x * 2; }

int test_main(void) {
    int a = 10, b = 20, c = 30;
    int d = double_it(a);  // 20, must preserve b, c
    int e = double_it(b);  // 40, must preserve a, c, d
    int f = double_it(c);  // 60, must preserve a, b, d, e
    return a + b + c + d + e + f;  // 10+20+30+20+40+60 = 180
}
