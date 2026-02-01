// INTEGRATION-TEST
// EXPECT: 120

// Deep call chain with value preservation
int add5(int x) { return x + 5; }
int mul2(int x) { return x * 2; }
int sub3(int x) { return x - 3; }

int test_main(void) {
    int a = 10;
    // Chain: 10 -> 15 -> 30 -> 27 -> 32 -> 64 -> 61 -> 66 -> 132 -> 129
    a = add5(a);  // 15
    a = mul2(a);  // 30
    a = sub3(a);  // 27
    a = add5(a);  // 32
    a = mul2(a);  // 64
    a = sub3(a);  // 61
    a = add5(a);  // 66
    a = mul2(a);  // 132
    a = sub3(a);  // 129
    return a - 9; // 120
}
