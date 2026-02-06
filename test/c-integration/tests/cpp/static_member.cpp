// INTEGRATION-TEST
// EXPECT: 30
// Test static member variable and function

class Counter {
    static int count;
public:
    Counter() { count++; }
    static int getCount() { return count; }
    static void reset() { count = 0; }
};

int Counter::count = 0;

extern "C" int test_main(void) {
    Counter::reset();

    Counter a;  // count = 1
    Counter b;  // count = 2
    Counter c;  // count = 3

    return Counter::getCount() * 10;  // 3 * 10 = 30
}
