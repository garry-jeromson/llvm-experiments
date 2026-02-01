// INTEGRATION-TEST
// EXPECT: 42

// Compound assignment operators
int test_main(void) {
    int a = 10;
    a += 5;   // 15
    a -= 3;   // 12
    a <<= 1;  // 24
    a |= 2;   // 26
    a += 16;  // 42
    return a;
}
