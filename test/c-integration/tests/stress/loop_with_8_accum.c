// INTEGRATION-TEST
// EXPECT: 180

// Loop with 8 accumulator variables
int test_main(void) {
    int s1 = 0, s2 = 0, s3 = 0, s4 = 0;
    int s5 = 0, s6 = 0, s7 = 0, s8 = 0;

    for (int i = 1; i <= 5; i++) {
        s1 += i;      // 15
        s2 += i * 2;  // 30
        s3 += i + 1;  // 20
        s4 += i - 1;  // 10
        s5 += 1;      // 5
        s6 += 2;      // 10
        s7 += i * i;  // 55
        s8 += 7;      // 35
    }
    return s1 + s2 + s3 + s4 + s5 + s6 + s7 + s8;  // 15+30+20+10+5+10+55+35 = 180
}
