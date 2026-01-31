// Unit tests for math module
#include "test_framework.hpp"
#include <snes/math.hpp>

using namespace snes;
using namespace snes::math;

// Angle tests
TEST(angle_default) {
    Angle a;
    ASSERT_EQ(a.raw, 0);
}

TEST(angle_from_degrees) {
    auto a90 = Angle::from_degrees(90);
    ASSERT_EQ(a90.raw, 64);  // 90 degrees = 64/256 of circle

    auto a180 = Angle::from_degrees(180);
    ASSERT_EQ(a180.raw, 128);

    auto a360 = Angle::from_degrees(360);
    ASSERT_EQ(a360.raw, 0);  // Wraps around
}

TEST(angle_arithmetic) {
    Angle a(32);
    Angle b(16);

    auto sum = a + b;
    ASSERT_EQ(sum.raw, 48);

    auto diff = a - b;
    ASSERT_EQ(diff.raw, 16);
}

TEST(angle_wrap) {
    Angle a(200);
    Angle b(100);
    auto sum = a + b;
    ASSERT_EQ(sum.raw, 44);  // (200 + 100) % 256 = 44
}

// Trig tests
TEST(sin_zero) {
    auto s = sin(Angle(0));
    ASSERT_EQ(s.raw, 0);  // sin(0) = 0
}

TEST(sin_90) {
    auto s = sin(Angle(64));  // 90 degrees
    ASSERT_EQ(s.raw, 256);  // sin(90) = 1.0 = 256 in 8.8
}

TEST(sin_180) {
    auto s = sin(Angle(128));  // 180 degrees
    ASSERT_EQ(s.raw, 0);  // sin(180) = 0
}

TEST(sin_270) {
    auto s = sin(Angle(192));  // 270 degrees
    ASSERT_EQ(s.raw, -256);  // sin(270) = -1.0
}

TEST(cos_zero) {
    auto c = cos(Angle(0));
    ASSERT_EQ(c.raw, 256);  // cos(0) = 1.0
}

TEST(cos_90) {
    auto c = cos(Angle(64));  // 90 degrees
    ASSERT_EQ(c.raw, 0);  // cos(90) = 0
}

TEST(cos_180) {
    auto c = cos(Angle(128));  // 180 degrees
    ASSERT_EQ(c.raw, -256);  // cos(180) = -1.0
}

// Min/Max/Clamp tests
TEST(min_int) {
    ASSERT_EQ(min(3, 5), 3);
    ASSERT_EQ(min(5, 3), 3);
    ASSERT_EQ(min(-1, 1), -1);
}

TEST(max_int) {
    ASSERT_EQ(max(3, 5), 5);
    ASSERT_EQ(max(5, 3), 5);
    ASSERT_EQ(max(-1, 1), 1);
}

TEST(clamp_int) {
    ASSERT_EQ(clamp(5, 0, 10), 5);   // Within range
    ASSERT_EQ(clamp(-5, 0, 10), 0);  // Below min
    ASSERT_EQ(clamp(15, 0, 10), 10); // Above max
}

TEST(abs_int) {
    ASSERT_EQ(abs(5), 5);
    ASSERT_EQ(abs(-5), 5);
    ASSERT_EQ(abs(0), 0);
}

TEST(sign_int) {
    ASSERT_EQ(sign(5), 1);
    ASSERT_EQ(sign(-5), -1);
    ASSERT_EQ(sign(0), 0);
}

// Lerp tests
TEST(lerp_i16) {
    ASSERT_EQ(lerp(static_cast<i16>(0), static_cast<i16>(100), 0), 0);
    ASSERT_EQ(lerp(static_cast<i16>(0), static_cast<i16>(100), 128), 50);
    ASSERT_EQ(lerp(static_cast<i16>(0), static_cast<i16>(100), 255), 99);  // Not quite 100 due to integer math
}

TEST(lerp_fixed8) {
    auto a = Fixed8::from_int(0);
    auto b = Fixed8::from_int(100);
    auto mid = lerp(a, b, 128);
    ASSERT_EQ(mid.to_int(), 50);
}

// Distance squared test
TEST(dist_sq_basic) {
    ASSERT_EQ(dist_sq(0, 0, 3, 4), 25);  // 3^2 + 4^2 = 25
    ASSERT_EQ(dist_sq(0, 0, 0, 0), 0);
    ASSERT_EQ(dist_sq(1, 1, 4, 5), 25);  // (4-1)^2 + (5-1)^2 = 9 + 16 = 25
}

// Random tests
TEST(random_next) {
    Random rng(12345);
    u16 a = rng.next();
    u16 b = rng.next();
    ASSERT_NE(a, b);  // Very unlikely to be the same
}

TEST(random_range) {
    Random rng(42);
    for (int i = 0; i < 100; i++) {
        u16 val = rng.range(10);
        ASSERT_LT(val, 10u);
    }
}

TEST(random_range_minmax) {
    Random rng(42);
    for (int i = 0; i < 100; i++) {
        u16 val = rng.range(5, 10);
        ASSERT_GE(val, 5u);
        ASSERT_LT(val, 10u);
    }
}

TEST(random_seed_reproducible) {
    Random rng1(999);
    Random rng2(999);

    for (int i = 0; i < 10; i++) {
        ASSERT_EQ(rng1.next(), rng2.next());
    }
}

TEST(random_zero_seed_handled) {
    Random rng(0);  // Should not break
    u16 a = rng.next();
    u16 b = rng.next();
    ASSERT_NE(a, 0u);  // Shouldn't be stuck at 0
    ASSERT_NE(a, b);
}
