#pragma once

// Freestanding environment - define types without standard library

namespace snes {

// Basic integer types (W65816 is 16-bit, little-endian)
using u8  = unsigned char;
using u16 = unsigned short;
using u32 = unsigned long;
using i8  = signed char;
using i16 = signed short;
using i32 = signed long;

// 8.8 fixed point number
struct Fixed8 {
    i16 raw;

    constexpr Fixed8() : raw(0) {}
    constexpr explicit Fixed8(i16 r) : raw(r) {}

    constexpr static Fixed8 from_int(int v) {
        return Fixed8(static_cast<i16>(v << 8));
    }

    constexpr static Fixed8 from_float(float v) {
        return Fixed8(static_cast<i16>(v * 256.0f));
    }

    constexpr int to_int() const { return raw >> 8; }

    // Fractional part (0-255)
    constexpr u8 frac() const { return static_cast<u8>(raw & 0xFF); }

    constexpr Fixed8 operator+(Fixed8 o) const {
        return Fixed8(static_cast<i16>(raw + o.raw));
    }

    constexpr Fixed8 operator-(Fixed8 o) const {
        return Fixed8(static_cast<i16>(raw - o.raw));
    }

    constexpr Fixed8 operator-() const {
        return Fixed8(static_cast<i16>(-raw));
    }

    Fixed8 operator*(Fixed8 o) const {
        return Fixed8(static_cast<i16>((static_cast<i32>(raw) * o.raw) >> 8));
    }

    Fixed8 operator/(Fixed8 o) const {
        return Fixed8(static_cast<i16>((static_cast<i32>(raw) << 8) / o.raw));
    }

    constexpr Fixed8& operator+=(Fixed8 o) {
        raw = static_cast<i16>(raw + o.raw);
        return *this;
    }

    constexpr Fixed8& operator-=(Fixed8 o) {
        raw = static_cast<i16>(raw - o.raw);
        return *this;
    }

    constexpr bool operator==(Fixed8 o) const { return raw == o.raw; }
    constexpr bool operator!=(Fixed8 o) const { return raw != o.raw; }
    constexpr bool operator<(Fixed8 o) const { return raw < o.raw; }
    constexpr bool operator<=(Fixed8 o) const { return raw <= o.raw; }
    constexpr bool operator>(Fixed8 o) const { return raw > o.raw; }
    constexpr bool operator>=(Fixed8 o) const { return raw >= o.raw; }
};

// 4.12 fixed point for higher precision (useful for angles/trig)
struct Fixed12 {
    i16 raw;

    constexpr Fixed12() : raw(0) {}
    constexpr explicit Fixed12(i16 r) : raw(r) {}

    constexpr static Fixed12 from_int(int v) {
        return Fixed12(static_cast<i16>(v << 12));
    }

    constexpr int to_int() const { return raw >> 12; }

    constexpr Fixed12 operator+(Fixed12 o) const {
        return Fixed12(static_cast<i16>(raw + o.raw));
    }

    constexpr Fixed12 operator-(Fixed12 o) const {
        return Fixed12(static_cast<i16>(raw - o.raw));
    }
};

// BGR555 color (native SNES format)
struct Color {
    u16 raw;

    constexpr Color() : raw(0) {}
    constexpr explicit Color(u16 r) : raw(r) {}

    constexpr static Color from_rgb(u8 r, u8 g, u8 b) {
        return Color(static_cast<u16>(
            ((r & 0x1F)) |
            ((g & 0x1F) << 5) |
            ((b & 0x1F) << 10)
        ));
    }

    constexpr u8 red() const { return static_cast<u8>(raw & 0x1F); }
    constexpr u8 green() const { return static_cast<u8>((raw >> 5) & 0x1F); }
    constexpr u8 blue() const { return static_cast<u8>((raw >> 10) & 0x1F); }
};

// 2D vector with fixed-point components
struct Vec2 {
    Fixed8 x;
    Fixed8 y;

    constexpr Vec2() : x(), y() {}
    constexpr Vec2(Fixed8 x_, Fixed8 y_) : x(x_), y(y_) {}
    constexpr Vec2(int x_, int y_) : x(Fixed8::from_int(x_)), y(Fixed8::from_int(y_)) {}

    constexpr Vec2 operator+(Vec2 o) const { return Vec2(x + o.x, y + o.y); }
    constexpr Vec2 operator-(Vec2 o) const { return Vec2(x - o.x, y - o.y); }
    constexpr Vec2 operator-() const { return Vec2(-x, -y); }
};

// Rectangle
struct Rect {
    i16 x;
    i16 y;
    u16 width;
    u16 height;

    constexpr Rect() : x(0), y(0), width(0), height(0) {}
    constexpr Rect(i16 x_, i16 y_, u16 w, u16 h) : x(x_), y(y_), width(w), height(h) {}

    constexpr i16 left() const { return x; }
    constexpr i16 right() const { return static_cast<i16>(x + width); }
    constexpr i16 top() const { return y; }
    constexpr i16 bottom() const { return static_cast<i16>(y + height); }

    constexpr bool contains(i16 px, i16 py) const {
        return px >= x && px < right() && py >= y && py < bottom();
    }
};

} // namespace snes
