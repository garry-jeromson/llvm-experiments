// INTEGRATION-TEST
// EXPECT: 99

// Pointer to pointer
int test_main(void) {
    int val = 99;
    int *ptr = &val;
    int **pptr = &ptr;
    return **pptr;
}
