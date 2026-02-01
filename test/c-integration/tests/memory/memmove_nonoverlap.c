// INTEGRATION-TEST
// EXPECT: 60

// memmove with non-overlapping regions (same as memcpy)
void *memmove(void *dest, const void *src, unsigned int n);

int test_main(void) {
    int src[3] = {10, 20, 30};
    int dest[3] = {0, 0, 0};

    memmove(dest, src, 6);  // Copy 3 ints

    return dest[0] + dest[1] + dest[2];  // 10 + 20 + 30 = 60
}
