// INTEGRATION-TEST
// EXPECT: 100

// 4 live values - should stress the 3-register limit
int test_main(void) {
    int a = 10;
    int b = 20;
    int c = 30;
    int d = 40;
    return a + b + c + d;
}
