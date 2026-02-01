// INTEGRATION-TEST
// EXPECT: 150

// Multiple struct field accesses creating pressure
typedef struct {
    int a, b, c, d, e;
} FiveFields;

int test_main(void) {
    FiveFields s;
    s.a = 10;
    s.b = 20;
    s.c = 30;
    s.d = 40;
    s.e = 50;
    // All fields accessed together
    return s.a + s.b + s.c + s.d + s.e;
}
