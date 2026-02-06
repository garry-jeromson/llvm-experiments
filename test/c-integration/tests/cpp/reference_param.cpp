// INTEGRATION-TEST
// EXPECT: 50
// Test pass by reference

void add_ten(int &x) {
    x += 10;
}

extern "C" int test_main(void) {
    int val = 30;
    add_ten(val);
    add_ten(val);
    return val;
}
