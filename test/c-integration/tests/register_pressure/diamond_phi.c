// INTEGRATION-TEST
// EXPECT: 80

// Diamond control flow with multiple phi nodes
int test_main(void) {
    int a = 10;
    int b = 20;
    int c = 30;

    if (a > 5) {
        a = a + 10;  // 20
        b = b + 10;  // 30
    } else {
        a = a - 10;
        b = b - 10;
    }
    // Phi nodes for both a and b here
    return a + b + c;  // 20 + 30 + 30 = 80
}
