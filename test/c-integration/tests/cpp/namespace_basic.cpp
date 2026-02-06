// INTEGRATION-TEST
// EXPECT: 30
// Test namespaces

namespace math {
    int add(int a, int b) {
        return a + b;
    }

    int multiply(int a, int b) {
        return a * b;
    }
}

namespace util {
    int double_it(int x) {
        return x * 2;
    }
}

extern "C" int test_main(void) {
    int sum = math::add(5, 10);  // 15
    int doubled = util::double_it(sum);  // 30
    return doubled;
}
