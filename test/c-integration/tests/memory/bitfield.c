// INTEGRATION-TEST
// EXPECT: 147
// Test C bitfields

struct Flags {
    unsigned int a : 4;  // 4 bits (0-15)
    unsigned int b : 4;  // 4 bits (0-15)
    unsigned int c : 8;  // 8 bits (0-255)
};

int test_main(void) {
    struct Flags f;
    f.a = 7;    // Store 7 in 4 bits
    f.b = 10;   // Store 10 in 4 bits
    f.c = 130;  // Store 130 in 8 bits

    // Verify values
    if (f.a != 7) return 200;
    if (f.b != 10) return 201;
    if (f.c != 130) return 202;

    return f.a + f.b + f.c;  // 7 + 10 + 130 = 147
}
