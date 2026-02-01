// INTEGRATION-TEST
// EXPECT: 100

int global_var = 50;

int test_main(void) {
    global_var = global_var + 50;
    return global_var;  // 100
}
