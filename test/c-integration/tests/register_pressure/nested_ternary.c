// INTEGRATION-TEST
// EXPECT: 30

// Nested ternary operators (multiple selects)
int test_main(void) {
    int a = 1, b = 2, c = 3;
    // Nested selects create complex phi patterns
    int result = a > 0 ? (b > 1 ? (c > 2 ? 30 : 20) : 10) : 0;
    return result;
}
