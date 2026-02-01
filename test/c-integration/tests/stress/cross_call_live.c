// INTEGRATION-TEST
// EXPECT: 450

// Multiple live values across function calls
int double_it(int x) { return x * 2; }

int test_main(void) {
    int a = 10, b = 20, c = 30, d = 40, e = 50;

    // Each call must preserve the other variables
    int r1 = double_it(a);  // 20
    int r2 = double_it(b);  // 40
    int r3 = double_it(c);  // 60
    int r4 = double_it(d);  // 80
    int r5 = double_it(e);  // 100

    // All original values still needed
    return a + b + c + d + e + r1 + r2 + r3 + r4 + r5;
    // 10+20+30+40+50 + 20+40+60+80+100 = 150 + 300 = 450
}
