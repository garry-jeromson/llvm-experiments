// INTEGRATION-TEST
// EXPECT: 42
// Test destructor

int destructor_value = 0;

class Resource {
    int val;
public:
    Resource(int v) : val(v) {}
    ~Resource() {
        destructor_value = val;
    }
    int get() { return val; }
};

extern "C" int test_main(void) {
    {
        Resource r(42);
        // r goes out of scope here, destructor called
    }
    return destructor_value;
}
