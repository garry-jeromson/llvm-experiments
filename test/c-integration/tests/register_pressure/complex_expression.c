// INTEGRATION-TEST
// EXPECT: 100

// Complex expression requiring many temporaries
int test_main(void) {
    int a = 2, b = 3, c = 4, d = 5;
    // Expression tree with many intermediate values
    int result = ((a + b) * (c + d)) + ((a * b) + (c * d)) - ((a - b) * (c - d));
    // (2+3)*(4+5) + (2*3)+(4*5) - (2-3)*(4-5)
    // 5*9 + 6+20 - (-1)*(-1)
    // 45 + 26 - 1 = 70
    return result + 30;
}
