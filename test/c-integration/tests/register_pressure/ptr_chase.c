// INTEGRATION-TEST
// EXPECT: 42

// Pointer chasing with multiple levels
int test_main(void) {
    int val = 42;
    int *p1 = &val;
    int **p2 = &p1;
    int ***p3 = &p2;

    // Dereference chain
    return ***p3;
}
