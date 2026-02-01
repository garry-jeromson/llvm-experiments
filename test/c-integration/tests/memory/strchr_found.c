// INTEGRATION-TEST
// EXPECT: 1

// Test strchr - find character in string

char *strchr(const char *s, int c);

int test_main(void) {
    const char *str = "hello";
    char *result = strchr(str, 'l');
    // 'l' is at index 2, so result should be str + 2
    // Return 1 if found at correct position
    if (result == 0) return 0;
    return (result - str == 2) ? 1 : 0;
}
