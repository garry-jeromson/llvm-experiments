// INTEGRATION-TEST
// EXPECT: 42
// Test basic class with member functions

class Counter {
    int value;
public:
    void set(int v) { value = v; }
    int get() { return value; }
    void add(int n) { value += n; }
};

extern "C" int test_main(void) {
    Counter c;
    c.set(30);
    c.add(12);
    return c.get();
}
