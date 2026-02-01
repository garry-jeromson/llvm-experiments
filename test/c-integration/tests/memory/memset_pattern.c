// INTEGRATION-TEST
// EXPECT: 170

// memset with non-zero pattern (0xAA = 170)
void *memset(void *s, int c, unsigned int n);

int test_main(void) {
    unsigned char arr[4] = {0, 0, 0, 0};

    memset(arr, 0xAA, 1);  // Set first byte to 0xAA

    return arr[0];  // 170
}
