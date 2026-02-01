// INTEGRATION-TEST
// EXPECT: 0

// Test strlen - empty string

int strlen(const char *s);

int test_main(void) {
    return strlen("");
}
