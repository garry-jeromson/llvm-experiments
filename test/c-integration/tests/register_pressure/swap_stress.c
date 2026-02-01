// INTEGRATION-TEST
// EXPECT: 132

// Multiple swaps stressing register allocation
int test_main(void) {
    int a = 1, b = 2, c = 3;
    int tmp;

    // Swap a and b
    tmp = a; a = b; b = tmp;  // a=2, b=1

    // Swap b and c
    tmp = b; b = c; c = tmp;  // b=3, c=1

    // Swap a and c
    tmp = a; a = c; c = tmp;  // a=1, c=2

    // Start: a=1, b=2, c=3
    // Swap a,b: a=2, b=1, c=3
    // Swap b,c: a=2, b=3, c=1
    // Swap a,c: a=1, b=3, c=2
    // Result: 1*100 + 3*10 + 2 = 132
    return a * 100 + b * 10 + c;
}
