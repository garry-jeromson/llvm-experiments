#pragma once

// SNES Math API - Header-only math utilities for W65816 backend
// Includes angle/trig functions, min/max/clamp, lerp, random number generation

#include "types.hpp"

namespace snes {
namespace math {

// ============================================================================
// Angle (256-unit circle, 1 byte = full rotation)
// ============================================================================

struct Angle {
    u8 raw;

    constexpr Angle() : raw(0) {}
    constexpr explicit Angle(u8 r) : raw(r) {}

    // Create angle from degrees (0-360)
    static constexpr Angle from_degrees(int deg) {
        // 360 degrees = 256 units, so deg * 256 / 360 = deg * 32 / 45
        // Use 256/360 approximation: multiply by 256, divide by 360
        // Simplified: (deg * 256 + 180) / 360 for rounding
        return Angle(static_cast<u8>((deg * 256 / 360) & 0xFF));
    }

    constexpr Angle operator+(Angle o) const {
        return Angle(static_cast<u8>((raw + o.raw) & 0xFF));
    }

    constexpr Angle operator-(Angle o) const {
        return Angle(static_cast<u8>((raw - o.raw) & 0xFF));
    }

    constexpr Angle& operator+=(Angle o) {
        raw = static_cast<u8>((raw + o.raw) & 0xFF);
        return *this;
    }

    constexpr Angle& operator-=(Angle o) {
        raw = static_cast<u8>((raw - o.raw) & 0xFF);
        return *this;
    }

    constexpr bool operator==(Angle o) const { return raw == o.raw; }
    constexpr bool operator!=(Angle o) const { return raw != o.raw; }
};

// ============================================================================
// Sine/Cosine Lookup Tables (256 entries, 8.8 fixed point)
// ============================================================================

// Sine table for quarter circle (64 entries), values in 8.8 fixed point
// sin(0) to sin(90 degrees) mapped to 0-64 indices
// Full table would be 256 entries but we use symmetry
namespace detail {

// Quarter sine table (0-64, representing 0-90 degrees)
// Values are 8.8 fixed point (256 = 1.0)
constexpr i16 sin_quarter[65] = {
    0,    6,   12,   18,   25,   31,   37,   43,   49,   56,   62,   68,   74,   80,   86,   92,
    97,  103,  109,  115,  120,  126,  131,  136,  142,  147,  152,  157,  162,  167,  171,  176,
   181,  185,  189,  193,  197,  201,  205,  209,  212,  216,  219,  222,  225,  228,  231,  234,
   236,  238,  241,  243,  245,  247,  248,  250,  251,  252,  253,  254,  255,  255,  256,  256,
   256
};

} // namespace detail

// Sin function using lookup table (returns 8.8 fixed point)
inline Fixed8 sin(Angle a) {
    u8 idx = a.raw;
    i16 val;

    if (idx < 64) {
        // First quadrant: 0-63 -> sin is positive, increasing
        val = detail::sin_quarter[idx];
    } else if (idx < 128) {
        // Second quadrant: 64-127 -> sin is positive, decreasing
        val = detail::sin_quarter[128 - idx];
    } else if (idx < 192) {
        // Third quadrant: 128-191 -> sin is negative, increasing magnitude
        val = -detail::sin_quarter[idx - 128];
    } else {
        // Fourth quadrant: 192-255 -> sin is negative, decreasing magnitude
        val = -detail::sin_quarter[256 - idx];
    }

    return Fixed8(val);
}

// Cos function (sin shifted by 90 degrees)
inline Fixed8 cos(Angle a) {
    return sin(Angle(static_cast<u8>((a.raw + 64) & 0xFF)));
}

// ============================================================================
// Min / Max / Clamp
// ============================================================================

template<typename T>
constexpr T min(T a, T b) {
    return (a < b) ? a : b;
}

template<typename T>
constexpr T max(T a, T b) {
    return (a > b) ? a : b;
}

template<typename T>
constexpr T clamp(T val, T lo, T hi) {
    return (val < lo) ? lo : ((val > hi) ? hi : val);
}

// ============================================================================
// Abs / Sign
// ============================================================================

constexpr i16 abs(i16 v) {
    return (v < 0) ? static_cast<i16>(-v) : v;
}

constexpr i16 sign(i16 v) {
    if (v > 0) return 1;
    if (v < 0) return -1;
    return 0;
}

// ============================================================================
// Linear Interpolation
// ============================================================================

// Lerp between two values
// t: 0-255 (0 = a, 255 = almost b, 256 would be exactly b)
template<typename T>
constexpr T lerp(T a, T b, u8 t) {
    // a + (b - a) * t / 256
    // For integer types, use i32 intermediate to avoid overflow
    i32 diff = static_cast<i32>(b) - static_cast<i32>(a);
    return static_cast<T>(static_cast<i32>(a) + (diff * t) / 256);
}

// Specialization for Fixed8
inline Fixed8 lerp(Fixed8 a, Fixed8 b, u8 t) {
    i32 diff = static_cast<i32>(b.raw) - static_cast<i32>(a.raw);
    return Fixed8(static_cast<i16>(a.raw + (diff * t) / 256));
}

// ============================================================================
// Distance (squared, to avoid sqrt)
// ============================================================================

// Distance squared between two points (avoids square root)
constexpr i32 dist_sq(i16 x1, i16 y1, i16 x2, i16 y2) {
    i32 dx = static_cast<i32>(x2) - static_cast<i32>(x1);
    i32 dy = static_cast<i32>(y2) - static_cast<i32>(y1);
    return dx * dx + dy * dy;
}

// ============================================================================
// Random Number Generator (LFSR-based)
// ============================================================================

class Random {
    u16 state;

public:
    // Construct with seed (0 is automatically replaced with non-zero)
    explicit Random(u16 seed = 1) : state(seed ? seed : 0xACE1) {}

    // Get next random value (full 16-bit range)
    u16 next() {
        // 16-bit LFSR with taps at 16, 14, 13, 11
        // Polynomial: x^16 + x^14 + x^13 + x^11 + 1
        u16 bit = ((state >> 0) ^ (state >> 2) ^ (state >> 3) ^ (state >> 5)) & 1;
        state = (state >> 1) | (bit << 15);
        return state;
    }

    // Get random value in range [0, max)
    u16 range(u16 max) {
        if (max == 0) return 0;
        return next() % max;
    }

    // Get random value in range [min, max)
    u16 range(u16 min_val, u16 max_val) {
        if (max_val <= min_val) return min_val;
        return min_val + (next() % (max_val - min_val));
    }

    // Reset with new seed
    void seed(u16 s) {
        state = s ? s : 0xACE1;
    }
};

} // namespace math

// Bring commonly used items into snes namespace for convenience
using math::Angle;
using math::Random;

} // namespace snes
