// INTEGRATION-TEST
// EXPECT: 50
void set_value(int *p, int val) {
    *p = val;
}

int test_main(void) {
    int x = 0;
    set_value(&x, 50);
    return x;
}
