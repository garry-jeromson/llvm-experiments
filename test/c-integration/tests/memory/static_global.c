// INTEGRATION-TEST
// EXPECT: 30
static int accumulator = 0;

void add_to(int x) {
    accumulator += x;
}

int test_main(void) {
    add_to(10);
    add_to(20);
    return accumulator;
}
