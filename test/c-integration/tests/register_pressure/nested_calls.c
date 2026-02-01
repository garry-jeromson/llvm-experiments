// INTEGRATION-TEST
// EXPECT: 150

// Multiple values live across calls
int double_it(int x) {
    return x + x;
}

int test_main(void) {
    int a = 10;
    int b = double_it(a);    // 20, a still live
    int c = double_it(b);    // 40, a,b still live
    int d = double_it(c);    // 80, a,b,c still live
    return a + b + c + d;    // 10 + 20 + 40 + 80 = 150
}
