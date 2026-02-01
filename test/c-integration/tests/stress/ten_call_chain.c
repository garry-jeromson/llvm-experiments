// INTEGRATION-TEST
// EXPECT: 100

// Ten-deep function call chain
int f1(int x) { return x + 1; }
int f2(int x) { return f1(x) + 1; }
int f3(int x) { return f2(x) + 1; }
int f4(int x) { return f3(x) + 1; }
int f5(int x) { return f4(x) + 1; }
int f6(int x) { return f5(x) + 1; }
int f7(int x) { return f6(x) + 1; }
int f8(int x) { return f7(x) + 1; }
int f9(int x) { return f8(x) + 1; }
int f10(int x) { return f9(x) + 1; }

int test_main(void) {
    return f10(90);  // 90 + 10 = 100
}
