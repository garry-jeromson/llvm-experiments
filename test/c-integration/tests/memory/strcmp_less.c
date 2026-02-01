// INTEGRATION-TEST
// EXPECT: 1

// Test strcmp - s1 < s2 returns negative (we return 1 if negative)

int strcmp(const char *s1, const char *s2);

int test_main(void) {
    int result = strcmp("abc", "abd");
    // 'c' (99) - 'd' (100) = -1, which is negative
    return result < 0 ? 1 : 0;
}
