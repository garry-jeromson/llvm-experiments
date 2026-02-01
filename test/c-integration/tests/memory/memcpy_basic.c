// INTEGRATION-TEST
// EXPECT: 123

// Basic memcpy test
void *memcpy(void *dest, const void *src, unsigned int n);

int test_main(void) {
    int src[3] = {100, 20, 3};
    int dest[3] = {0, 0, 0};

    memcpy(dest, src, 6);  // Copy 3 ints (6 bytes)

    return dest[0] + dest[1] + dest[2];  // 100 + 20 + 3 = 123
}
