// INTEGRATION-TEST
// EXPECT: 70

// Simulate indirect call pattern with if-else chain
int add(int a, int b) { return a + b; }
int sub(int a, int b) { return a - b; }
int mul(int a, int b) { return a * b; }

int test_main(void) {
    int result = 0;
    int x = 10, y = 5;

    // Simulate function pointer dispatch
    for (int op = 0; op < 3; op++) {
        if (op == 0) {
            result += add(x, y);  // 15
        } else if (op == 1) {
            result += sub(x, y);  // 5
        } else {
            result += mul(x, y);  // 50
        }
    }
    return result;  // 15 + 5 + 50 = 70
}
