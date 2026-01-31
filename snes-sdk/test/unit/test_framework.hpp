#pragma once

#include <cstdio>
#include <cstring>

// Minimal test framework for SDK unit tests
// No external dependencies

namespace test {

// Global test state
inline bool test_failed = false;
inline int total_tests = 0;
inline int passed_tests = 0;
inline int failed_tests = 0;

// Test registration and running
struct TestCase {
    const char* name;
    void (*func)();
    TestCase* next;

    static TestCase*& head() {
        static TestCase* h = nullptr;
        return h;
    }

    TestCase(const char* n, void (*f)()) : name(n), func(f), next(head()) {
        head() = this;
    }
};

// Assertion macros
#define ASSERT_TRUE(expr) \
    do { \
        if (!(expr)) { \
            std::printf("  FAIL: %s:%d: ASSERT_TRUE(%s)\n", __FILE__, __LINE__, #expr); \
            test::test_failed = true; \
        } \
    } while(0)

#define ASSERT_FALSE(expr) \
    do { \
        if ((expr)) { \
            std::printf("  FAIL: %s:%d: ASSERT_FALSE(%s)\n", __FILE__, __LINE__, #expr); \
            test::test_failed = true; \
        } \
    } while(0)

#define ASSERT_EQ(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (_a != _b) { \
            std::printf("  FAIL: %s:%d: ASSERT_EQ(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            test::test_failed = true; \
        } \
    } while(0)

#define ASSERT_NE(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (_a == _b) { \
            std::printf("  FAIL: %s:%d: ASSERT_NE(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            test::test_failed = true; \
        } \
    } while(0)

#define ASSERT_LT(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (!(_a < _b)) { \
            std::printf("  FAIL: %s:%d: ASSERT_LT(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            test::test_failed = true; \
        } \
    } while(0)

#define ASSERT_LE(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (!(_a <= _b)) { \
            std::printf("  FAIL: %s:%d: ASSERT_LE(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            test::test_failed = true; \
        } \
    } while(0)

#define ASSERT_GT(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (!(_a > _b)) { \
            std::printf("  FAIL: %s:%d: ASSERT_GT(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            test::test_failed = true; \
        } \
    } while(0)

#define ASSERT_GE(a, b) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        if (!(_a >= _b)) { \
            std::printf("  FAIL: %s:%d: ASSERT_GE(%s, %s)\n", __FILE__, __LINE__, #a, #b); \
            test::test_failed = true; \
        } \
    } while(0)

#define ASSERT_NEAR(a, b, epsilon) \
    do { \
        auto _a = (a); \
        auto _b = (b); \
        auto _e = (epsilon); \
        auto _diff = (_a > _b) ? (_a - _b) : (_b - _a); \
        if (_diff > _e) { \
            std::printf("  FAIL: %s:%d: ASSERT_NEAR(%s, %s, %s)\n", __FILE__, __LINE__, #a, #b, #epsilon); \
            test::test_failed = true; \
        } \
    } while(0)

// Test definition macro
#define TEST(name) \
    static void test_##name(); \
    static test::TestCase _test_case_##name(#name, test_##name); \
    static void test_##name()

// Test suite (for grouping)
#define TEST_SUITE(suite, name) \
    static void test_##suite##_##name(); \
    static test::TestCase _test_case_##suite##_##name(#suite "." #name, test_##suite##_##name); \
    static void test_##suite##_##name()

// Run all registered tests
inline int run_all_tests() {
    std::printf("Running tests...\n\n");

    // Count tests first
    int count = 0;
    for (TestCase* tc = TestCase::head(); tc; tc = tc->next) {
        count++;
    }

    // Run in reverse order (registration order)
    TestCase** tests = new TestCase*[count];
    int i = count - 1;
    for (TestCase* tc = TestCase::head(); tc; tc = tc->next) {
        tests[i--] = tc;
    }

    for (int j = 0; j < count; j++) {
        TestCase* tc = tests[j];
        total_tests++;
        test_failed = false;

        std::printf("  %s... ", tc->name);
        tc->func();

        if (test_failed) {
            failed_tests++;
        } else {
            passed_tests++;
            std::printf("OK\n");
        }
    }

    delete[] tests;

    std::printf("\n");
    std::printf("Results: %d passed, %d failed, %d total\n",
                passed_tests, failed_tests, total_tests);

    return failed_tests > 0 ? 1 : 0;
}

} // namespace test
