// INTEGRATION-TEST
// EXPECT: 42
int test_main(void) {
    int x = 10;
    if (x > 5) goto done;
    x = 100;
done:
    return x + 32;
}
