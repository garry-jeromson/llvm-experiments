// INTEGRATION-TEST
// EXPECT: 100

// Complex conditional expressions
int test_main(void) {
    int a = 5, b = 10, c = 15, d = 20;
    int result = 0;

    if ((a < b && c < d) || (a + b > c)) {
        result += 25;
    }
    if (!(a > b) && (c <= d)) {
        result += 25;
    }
    if ((a * 2 == b) && (d - c == 5)) {
        result += 25;
    }
    if ((a + b + c) < (d * 2)) {
        result += 25;
    }

    return result;  // 25 + 25 + 25 + 25 = 100
}
