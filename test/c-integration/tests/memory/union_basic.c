// INTEGRATION-TEST
// EXPECT: 255
typedef union {
    int word;
    unsigned char bytes[2];
} WordBytes;

int test_main(void) {
    WordBytes u;
    u.word = 0x12FF;
    return u.bytes[0];
}
