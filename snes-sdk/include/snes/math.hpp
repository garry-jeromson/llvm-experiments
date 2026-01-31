#pragma once

#include "types.hpp"

namespace snes::math {

// Re-export Fixed8 from types
using snes::Fixed8;
using snes::Fixed12;

// Angle type (256 = full rotation, so 1 unit = 1.40625 degrees)
struct Angle {
    u8 raw;

    constexpr Angle() : raw(0) {}
    constexpr explicit Angle(u8 r) : raw(r) {}

    // Named constructors
    constexpr static Angle from_degrees(int deg) {
        return Angle(static_cast<u8>((deg * 256) / 360));
    }

    constexpr static Angle from_radians(float rad) {
        return Angle(static_cast<u8>((rad * 256.0f) / (2.0f * 3.14159265f)));
    }

    // Arithmetic
    constexpr Angle operator+(Angle o) const { return Angle(static_cast<u8>(raw + o.raw)); }
    constexpr Angle operator-(Angle o) const { return Angle(static_cast<u8>(raw - o.raw)); }
    constexpr Angle operator-() const { return Angle(static_cast<u8>(-raw)); }

    constexpr Angle& operator+=(Angle o) { raw = static_cast<u8>(raw + o.raw); return *this; }
    constexpr Angle& operator-=(Angle o) { raw = static_cast<u8>(raw - o.raw); return *this; }

    // Comparison
    constexpr bool operator==(Angle o) const { return raw == o.raw; }
    constexpr bool operator!=(Angle o) const { return raw != o.raw; }
};

// Common angles
namespace angles {
    constexpr Angle ZERO = Angle(0);
    constexpr Angle DEG_45 = Angle(32);
    constexpr Angle DEG_90 = Angle(64);
    constexpr Angle DEG_135 = Angle(96);
    constexpr Angle DEG_180 = Angle(128);
    constexpr Angle DEG_225 = Angle(160);
    constexpr Angle DEG_270 = Angle(192);
    constexpr Angle DEG_315 = Angle(224);
}

// Sine/Cosine lookup tables (256 entries, 8.8 fixed point)
// sin(angle) where angle 0-255 = 0-360 degrees
// Values range from -256 to +256 (representing -1.0 to +1.0)
extern const i16 sin_table[256];
extern const i16 cos_table[256];

// Trigonometric functions (return 8.8 fixed point)
inline Fixed8 sin(Angle a) {
    return Fixed8(sin_table[a.raw]);
}

inline Fixed8 cos(Angle a) {
    return Fixed8(cos_table[a.raw]);
}

// Fast approximate functions
Fixed8 sin_fast(Angle a);  // Uses quarter-wave symmetry
Fixed8 cos_fast(Angle a);

// Utility templates
template<typename T>
constexpr T min(T a, T b) {
    return a < b ? a : b;
}

template<typename T>
constexpr T max(T a, T b) {
    return a > b ? a : b;
}

template<typename T>
constexpr T clamp(T v, T lo, T hi) {
    return min(max(v, lo), hi);
}

template<typename T>
constexpr T abs(T v) {
    return v < T(0) ? -v : v;
}

// Sign function (-1, 0, or 1)
template<typename T>
constexpr int sign(T v) {
    return (v > T(0)) - (v < T(0));
}

// Linear interpolation (0-256 = 0.0-1.0)
constexpr i16 lerp(i16 a, i16 b, u8 t) {
    return static_cast<i16>(a + (((b - a) * t) >> 8));
}

constexpr Fixed8 lerp(Fixed8 a, Fixed8 b, u8 t) {
    return Fixed8(lerp(a.raw, b.raw, t));
}

// Distance squared (avoids sqrt)
constexpr i32 dist_sq(i16 x1, i16 y1, i16 x2, i16 y2) {
    i32 dx = x2 - x1;
    i32 dy = y2 - y1;
    return dx * dx + dy * dy;
}

// Simple pseudo-random number generator (Linear Congruential Generator)
class Random {
    u16 m_seed;

public:
    explicit Random(u16 seed = 1) : m_seed(seed ? seed : 1) {}

    // Get next random value (full 16-bit range)
    u16 next() {
        // LCG with parameters from Numerical Recipes
        m_seed = static_cast<u16>(m_seed * 25173 + 13849);
        return m_seed;
    }

    // Get random value in range [0, max)
    u16 range(u16 max_val) {
        if (max_val == 0) return 0;
        return next() % max_val;
    }

    // Get random value in range [min, max)
    u16 range(u16 min_val, u16 max_val) {
        if (max_val <= min_val) return min_val;
        return min_val + range(max_val - min_val);
    }

    // Get random boolean
    bool coin_flip() {
        return (next() & 0x8000) != 0;
    }

    // Get random Fixed8 in range [0, 1)
    Fixed8 unit() {
        return Fixed8(static_cast<i16>(next() & 0xFF));
    }

    // Reseed
    void seed(u16 s) {
        m_seed = s ? s : 1;
    }
};

// Hardware multiply (uses SNES hardware multiply register)
// Performs: a * b where a is 8-bit unsigned, b is 8-bit unsigned
// Result is 16-bit
u16 hw_multiply(u8 a, u8 b);

// Hardware divide (uses SNES hardware divide register)
// Performs: a / b where a is 16-bit unsigned, b is 8-bit unsigned
// Returns quotient (16-bit) and remainder (16-bit) via pointers
void hw_divide(u16 a, u8 b, u16* quotient, u16* remainder);

// Integer square root (approximate, for distance calculations)
u16 isqrt(u32 n);

} // namespace snes::math
