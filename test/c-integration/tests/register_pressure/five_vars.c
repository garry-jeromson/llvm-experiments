// INTEGRATION-TEST
// EXPECT: 150

// 5 live values - definitely needs spilling
int test_main(void) {
    int a = 10;
    int b = 20;
    int c = 30;
    int d = 40;
    int e = 50;
    return a + b + c + d + e;
}
