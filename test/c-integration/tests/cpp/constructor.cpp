// INTEGRATION-TEST
// EXPECT: 100
// Test constructor

class Value {
    int val;
public:
    Value(int v) : val(v) {}
    int get() { return val; }
};

extern "C" int test_main(void) {
    Value v1(40);
    Value v2(60);
    return v1.get() + v2.get();
}
