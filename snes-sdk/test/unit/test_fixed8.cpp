// Unit tests for Fixed8 type
#include "test_framework.hpp"
#include <snes/types.hpp>

using namespace snes;

TEST(fixed8_default_constructor) {
    Fixed8 f;
    ASSERT_EQ(f.raw, 0);
}

TEST(fixed8_from_int) {
    auto f = Fixed8::from_int(5);
    ASSERT_EQ(f.raw, 5 << 8);  // 5 * 256 = 1280
    ASSERT_EQ(f.to_int(), 5);
}

TEST(fixed8_from_int_negative) {
    auto f = Fixed8::from_int(-3);
    ASSERT_EQ(f.to_int(), -3);
}

TEST(fixed8_from_float) {
    auto f = Fixed8::from_float(2.5f);
    ASSERT_EQ(f.to_int(), 2);
    ASSERT_EQ(f.frac(), 128);  // 0.5 * 256 = 128
}

TEST(fixed8_addition) {
    auto a = Fixed8::from_int(3);
    auto b = Fixed8::from_int(4);
    auto c = a + b;
    ASSERT_EQ(c.to_int(), 7);
}

TEST(fixed8_subtraction) {
    auto a = Fixed8::from_int(10);
    auto b = Fixed8::from_int(3);
    auto c = a - b;
    ASSERT_EQ(c.to_int(), 7);
}

TEST(fixed8_negation) {
    auto a = Fixed8::from_int(5);
    auto b = -a;
    ASSERT_EQ(b.to_int(), -5);
}

TEST(fixed8_multiplication) {
    auto a = Fixed8::from_int(3);
    auto b = Fixed8::from_int(4);
    auto c = a * b;
    ASSERT_EQ(c.to_int(), 12);
}

TEST(fixed8_multiplication_fractional) {
    // 1.5 * 2 = 3
    Fixed8 a(384);  // 1.5 in 8.8 = 1 * 256 + 0.5 * 256 = 384
    auto b = Fixed8::from_int(2);
    auto c = a * b;
    ASSERT_EQ(c.to_int(), 3);
}

TEST(fixed8_division) {
    auto a = Fixed8::from_int(12);
    auto b = Fixed8::from_int(3);
    auto c = a / b;
    ASSERT_EQ(c.to_int(), 4);
}

TEST(fixed8_comparison_equal) {
    auto a = Fixed8::from_int(5);
    auto b = Fixed8::from_int(5);
    ASSERT_TRUE(a == b);
    ASSERT_FALSE(a != b);
}

TEST(fixed8_comparison_less_than) {
    auto a = Fixed8::from_int(3);
    auto b = Fixed8::from_int(5);
    ASSERT_TRUE(a < b);
    ASSERT_TRUE(a <= b);
    ASSERT_FALSE(a > b);
    ASSERT_FALSE(a >= b);
}

TEST(fixed8_compound_assignment) {
    auto a = Fixed8::from_int(5);
    a += Fixed8::from_int(3);
    ASSERT_EQ(a.to_int(), 8);

    a -= Fixed8::from_int(2);
    ASSERT_EQ(a.to_int(), 6);
}

// Color tests
TEST(color_from_rgb) {
    auto c = Color::from_rgb(31, 0, 0);  // Pure red
    ASSERT_EQ(c.red(), 31);
    ASSERT_EQ(c.green(), 0);
    ASSERT_EQ(c.blue(), 0);
    ASSERT_EQ(c.raw, 0x001F);
}

TEST(color_from_rgb_white) {
    auto c = Color::from_rgb(31, 31, 31);  // White
    ASSERT_EQ(c.red(), 31);
    ASSERT_EQ(c.green(), 31);
    ASSERT_EQ(c.blue(), 31);
    ASSERT_EQ(c.raw, 0x7FFF);
}

TEST(color_from_rgb_blue) {
    auto c = Color::from_rgb(0, 0, 31);  // Pure blue
    ASSERT_EQ(c.raw, 0x7C00);  // Blue in bits 10-14
}

// Vec2 tests
TEST(vec2_default) {
    Vec2 v;
    ASSERT_EQ(v.x.raw, 0);
    ASSERT_EQ(v.y.raw, 0);
}

TEST(vec2_from_int) {
    Vec2 v(10, 20);
    ASSERT_EQ(v.x.to_int(), 10);
    ASSERT_EQ(v.y.to_int(), 20);
}

TEST(vec2_addition) {
    Vec2 a(3, 4);
    Vec2 b(1, 2);
    Vec2 c = a + b;
    ASSERT_EQ(c.x.to_int(), 4);
    ASSERT_EQ(c.y.to_int(), 6);
}

// Rect tests
TEST(rect_contains) {
    Rect r(10, 10, 20, 20);
    ASSERT_TRUE(r.contains(15, 15));
    ASSERT_TRUE(r.contains(10, 10));
    ASSERT_FALSE(r.contains(30, 15));  // Right edge is exclusive
    ASSERT_FALSE(r.contains(5, 15));
}

TEST(rect_bounds) {
    Rect r(10, 20, 30, 40);
    ASSERT_EQ(r.left(), 10);
    ASSERT_EQ(r.right(), 40);
    ASSERT_EQ(r.top(), 20);
    ASSERT_EQ(r.bottom(), 60);
}
