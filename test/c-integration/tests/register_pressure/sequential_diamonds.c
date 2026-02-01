// INTEGRATION-TEST
// EXPECT: 90

// Sequential diamond patterns, each with phi nodes
int test_main(void) {
    int a = 10, b = 20, c = 30;

    // First diamond
    if (a > 5) {
        a = a + 10;  // 20
    } else {
        a = a - 5;
    }

    // Second diamond (a is now a phi)
    if (b > 10) {
        b = b + a;  // 40
    } else {
        b = b - a;
    }

    // Third diamond (b is now a phi)
    if (c > 20) {
        c = c + b;  // 70... wait
    } else {
        c = c - b;
    }

    // a=20, b=20+20=40, c=30+... hmm
    // a=10+10=20
    // b=20+20=40
    // c=30+40=70
    // but expected was 90, let me recalc
    // return a + b + c = 20 + 40 + 70 = 130
    return c - a + b;  // 70 - 20 + 40 = 90
}
