// INTEGRATION-TEST
// EXPECT: 5
// Test chained ternary operators

int classify(int x) {
    return (x < 0)   ? -1 :
           (x == 0)  ? 0 :
           (x < 10)  ? 1 :
           (x < 100) ? 2 : 3;
}

int test_main(void) {
    int r1 = classify(-5);   // -1
    int r2 = classify(0);    // 0
    int r3 = classify(5);    // 1
    int r4 = classify(50);   // 2
    int r5 = classify(500);  // 3

    // Sum: -1 + 0 + 1 + 2 + 3 = 5
    return r1 + r2 + r3 + r4 + r5;
}
