// INTEGRATION-TEST
// EXPECT: 100

// Nested diamond CFG with multiple phi merge points
int test_main(void) {
    int a = 10, b = 20;
    int cond1 = 1, cond2 = 1;

    if (cond1) {
        if (cond2) {
            a = 30;
            b = 40;
        } else {
            a = 50;
            b = 60;
        }
        // Inner merge: 2 phi nodes
    } else {
        a = 70;
        b = 80;
    }
    // Outer merge: 2 more phi nodes (complex because inner already has phi)
    return a + b + a;  // 30 + 40 + 30 = 100
}
