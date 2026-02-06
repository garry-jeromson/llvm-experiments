// INTEGRATION-TEST
// EXPECT: 70
// Test operator overloading

class Number {
    int val;
public:
    Number(int v) : val(v) {}
    Number operator+(const Number &other) const {
        return Number(val + other.val);
    }
    int get() const { return val; }
};

extern "C" int test_main(void) {
    Number a(30);
    Number b(40);
    Number c = a + b;
    return c.get();
}
