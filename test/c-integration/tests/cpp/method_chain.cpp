// INTEGRATION-TEST
// EXPECT: 60
// Test method chaining (return *this)

class Builder {
    int value;
public:
    Builder() : value(0) {}

    Builder& add(int n) {
        value += n;
        return *this;
    }

    Builder& multiply(int n) {
        value *= n;
        return *this;
    }

    int get() const { return value; }
};

extern "C" int test_main(void) {
    Builder b;
    int result = b.add(5).add(5).multiply(3).add(30).get();
    // (0 + 5 + 5) * 3 + 30 = 30 + 30 = 60
    return result;
}
