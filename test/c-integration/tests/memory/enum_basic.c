// INTEGRATION-TEST
// EXPECT: 6
typedef enum {
    RED = 1,
    GREEN = 2,
    BLUE = 3
} Color;

int test_main(void) {
    Color c1 = RED;
    Color c2 = GREEN;
    Color c3 = BLUE;
    return c1 + c2 + c3;
}
