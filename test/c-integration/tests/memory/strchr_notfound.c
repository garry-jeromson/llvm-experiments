// INTEGRATION-TEST
// EXPECT: 1

// Test strchr - character not in string returns null

char *strchr(const char *s, int c);

int test_main(void) {
    char *result = strchr("hello", 'x');
    // 'x' is not in "hello", should return null (0)
    return (result == 0) ? 1 : 0;
}
