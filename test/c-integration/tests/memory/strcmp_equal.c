// INTEGRATION-TEST
// EXPECT: 0

// Test strcmp - equal strings return 0

int strcmp(const char *s1, const char *s2);

int test_main(void) {
    return strcmp("abc", "abc");
}
