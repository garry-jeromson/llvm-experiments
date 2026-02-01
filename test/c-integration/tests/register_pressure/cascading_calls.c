// INTEGRATION-TEST
// EXPECT: 100

// Cascading function calls with argument computation
int add(int x, int y) { return x + y; }
int mul(int x, int y) { return x * y; }

int test_main(void) {
    int a = 5, b = 10;
    // Arguments computed while preserving other values
    int r1 = add(a, b);           // 15
    int r2 = mul(a, b);           // 50
    int r3 = add(r1, r2);         // 65
    int r4 = add(a + b, r1);      // 30
    return r3 + r4 + a;           // 65 + 30 + 5 = 100
}
