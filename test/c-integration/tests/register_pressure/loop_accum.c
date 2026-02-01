// INTEGRATION-TEST
// EXPECT: 47

// Loop with multiple accumulators - phi nodes in loop header
int test_main(void) {
    int sum = 0;
    int doubled = 1;
    for (int i = 1; i <= 5; i++) {
        sum = sum + i;           // 1+2+3+4+5 = 15
        doubled = doubled + doubled;  // 1->2->4->8->16->32
    }
    return sum + doubled;  // 15 + 32 = 47
}
