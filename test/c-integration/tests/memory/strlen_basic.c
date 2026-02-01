// INTEGRATION-TEST
// EXPECT: 5

// Test strlen - basic string length

int strlen(const char *s);

int test_main(void) {
    // "hello" is 5 characters
    return strlen("hello");
}
