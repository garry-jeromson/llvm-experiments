// INTEGRATION-TEST
// EXPECT: 70
// SKIP: Function pointers not yet supported (encodeAddr16 limitation)
// Test function pointers

int add(int a, int b) {
    return a + b;
}

int multiply(int a, int b) {
    return a * b;
}

int apply(int (*func)(int, int), int x, int y) {
    return func(x, y);
}

int test_main(void) {
    int (*op)(int, int);

    op = add;
    int sum = op(10, 20);  // 30

    op = multiply;
    int product = op(5, 8);  // 40

    return sum + product;  // 70
}
