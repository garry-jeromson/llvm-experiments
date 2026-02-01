// INTEGRATION-TEST
// EXPECT: 32768

// INT16_MIN value (-32768 as unsigned = 32768)
int test_main(void) {
    int a = -32768;
    return a;
}
