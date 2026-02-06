// INTEGRATION-TEST
// EXPECT: 4660
// Regression test for AND/OR/XOR16rr when Src2=A and Src1!=A
// This pattern was broken: (a & 0xFF) | (b << 8)
// The OR expansion would overwrite A before using it.

static volatile int va, vb;

__attribute__((noinline))
int bitops_or(int a, int b) {
    // (a & 0xFF) | (b << 8)
    // Tests OR16rr with Src2=A (result of AND), Src1=X (result of SHL)
    return (a & 0xFF) | (b << 8);
}

int test_main(void) {
    va = 0x34;
    vb = 0x12;

    // Test OR: (0x34 & 0xFF) | (0x12 << 8) = 0x34 | 0x1200 = 0x1234 = 4660
    return bitops_or(va, vb);
}
