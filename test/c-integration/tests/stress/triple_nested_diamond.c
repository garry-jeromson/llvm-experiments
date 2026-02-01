// INTEGRATION-TEST
// EXPECT: 80

// Triple-nested diamond CFG
int test_main(void) {
    int a = 10, b = 20;
    int c1 = 1, c2 = 1, c3 = 1;

    if (c1) {
        if (c2) {
            if (c3) {
                a = 30;
                b = 50;
            } else {
                a = 40;
                b = 60;
            }
        } else {
            a = 50;
            b = 70;
        }
    } else {
        a = 60;
        b = 80;
    }
    return a + b;  // 30 + 50 = 80
}
