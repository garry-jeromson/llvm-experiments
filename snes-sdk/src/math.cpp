#include <snes/math.hpp>
#include <snes/hal.hpp>
#include <snes/registers.hpp>

namespace snes::math {

// Sine table (256 entries, 8.8 fixed point)
// sin(i * 360 / 256) * 256 for i = 0 to 255
const i16 sin_table[256] = {
       0,    6,   12,   18,   25,   31,   37,   43,
      49,   56,   62,   68,   74,   80,   86,   92,
      97,  103,  109,  115,  120,  126,  131,  136,
     142,  147,  152,  157,  162,  166,  171,  176,
     180,  185,  189,  193,  197,  201,  205,  208,
     212,  215,  219,  222,  225,  228,  231,  233,
     236,  238,  240,  242,  244,  246,  247,  249,
     250,  251,  252,  253,  254,  254,  255,  255,
     256,  255,  255,  254,  254,  253,  252,  251,
     250,  249,  247,  246,  244,  242,  240,  238,
     236,  233,  231,  228,  225,  222,  219,  215,
     212,  208,  205,  201,  197,  193,  189,  185,
     180,  176,  171,  166,  162,  157,  152,  147,
     142,  136,  131,  126,  120,  115,  109,  103,
      97,   92,   86,   80,   74,   68,   62,   56,
      49,   43,   37,   31,   25,   18,   12,    6,
       0,   -6,  -12,  -18,  -25,  -31,  -37,  -43,
     -49,  -56,  -62,  -68,  -74,  -80,  -86,  -92,
     -97, -103, -109, -115, -120, -126, -131, -136,
    -142, -147, -152, -157, -162, -166, -171, -176,
    -180, -185, -189, -193, -197, -201, -205, -208,
    -212, -215, -219, -222, -225, -228, -231, -233,
    -236, -238, -240, -242, -244, -246, -247, -249,
    -250, -251, -252, -253, -254, -254, -255, -255,
    -256, -255, -255, -254, -254, -253, -252, -251,
    -250, -249, -247, -246, -244, -242, -240, -238,
    -236, -233, -231, -228, -225, -222, -219, -215,
    -212, -208, -205, -201, -197, -193, -189, -185,
    -180, -176, -171, -166, -162, -157, -152, -147,
    -142, -136, -131, -126, -120, -115, -109, -103,
     -97,  -92,  -86,  -80,  -74,  -68,  -62,  -56,
     -49,  -43,  -37,  -31,  -25,  -18,  -12,   -6
};

// Cosine table (256 entries, 8.8 fixed point)
// cos(i * 360 / 256) * 256 for i = 0 to 255
// This is just sin shifted by 64 (90 degrees)
const i16 cos_table[256] = {
     256,  255,  255,  254,  254,  253,  252,  251,
     250,  249,  247,  246,  244,  242,  240,  238,
     236,  233,  231,  228,  225,  222,  219,  215,
     212,  208,  205,  201,  197,  193,  189,  185,
     180,  176,  171,  166,  162,  157,  152,  147,
     142,  136,  131,  126,  120,  115,  109,  103,
      97,   92,   86,   80,   74,   68,   62,   56,
      49,   43,   37,   31,   25,   18,   12,    6,
       0,   -6,  -12,  -18,  -25,  -31,  -37,  -43,
     -49,  -56,  -62,  -68,  -74,  -80,  -86,  -92,
     -97, -103, -109, -115, -120, -126, -131, -136,
    -142, -147, -152, -157, -162, -166, -171, -176,
    -180, -185, -189, -193, -197, -201, -205, -208,
    -212, -215, -219, -222, -225, -228, -231, -233,
    -236, -238, -240, -242, -244, -246, -247, -249,
    -250, -251, -252, -253, -254, -254, -255, -255,
    -256, -255, -255, -254, -254, -253, -252, -251,
    -250, -249, -247, -246, -244, -242, -240, -238,
    -236, -233, -231, -228, -225, -222, -219, -215,
    -212, -208, -205, -201, -197, -193, -189, -185,
    -180, -176, -171, -166, -162, -157, -152, -147,
    -142, -136, -131, -126, -120, -115, -109, -103,
     -97,  -92,  -86,  -80,  -74,  -68,  -62,  -56,
     -49,  -43,  -37,  -31,  -25,  -18,  -12,   -6,
       0,    6,   12,   18,   25,   31,   37,   43,
      49,   56,   62,   68,   74,   80,   86,   92,
      97,  103,  109,  115,  120,  126,  131,  136,
     142,  147,  152,  157,  162,  166,  171,  176,
     180,  185,  189,  193,  197,  201,  205,  208,
     212,  215,  219,  222,  225,  228,  231,  233,
     236,  238,  240,  242,  244,  246,  247,  249,
     250,  251,  252,  253,  254,  254,  255,  255
};

// Fast sine using quarter-wave symmetry
// Only needs 64-entry table but we have the full table anyway
Fixed8 sin_fast(Angle a) {
    return Fixed8(sin_table[a.raw]);
}

Fixed8 cos_fast(Angle a) {
    return Fixed8(cos_table[a.raw]);
}

// Hardware multiply using SNES registers
u16 hw_multiply(u8 a, u8 b) {
    // Write multiplicand A to $4202
    hal::write8(reg::WRMPYA::address, a);

    // Write multiplicand B to $4203 (triggers multiplication)
    hal::write8(reg::WRMPYB::address, b);

    // Wait 8 machine cycles (the write above takes some time)
    // On real hardware, a few NOPs would suffice
    // The result is available immediately after the instruction completes
    // due to SNES timing, but we read anyway

    // Read result from $4216-$4217
    u8 lo = hal::read8(reg::RDMPYL::address);
    u8 hi = hal::read8(reg::RDMPYH::address);

    return static_cast<u16>(lo | (hi << 8));
}

// Hardware divide using SNES registers
void hw_divide(u16 a, u8 b, u16* quotient, u16* remainder) {
    if (b == 0) {
        // Division by zero - return max values
        if (quotient) *quotient = 0xFFFF;
        if (remainder) *remainder = a;
        return;
    }

    // Write dividend to $4204-$4205
    hal::write8(reg::WRDIVL::address, static_cast<u8>(a & 0xFF));
    hal::write8(reg::WRDIVH::address, static_cast<u8>(a >> 8));

    // Write divisor to $4206 (triggers division)
    hal::write8(reg::WRDIVB::address, b);

    // Division takes 16 machine cycles
    // On real hardware, we'd need to wait or do other work
    // The result registers are:
    // $4214-$4215 = quotient
    // $4216-$4217 = remainder

    if (quotient) {
        u8 lo = hal::read8(reg::RDDIVL::address);
        u8 hi = hal::read8(reg::RDDIVH::address);
        *quotient = static_cast<u16>(lo | (hi << 8));
    }

    if (remainder) {
        u8 lo = hal::read8(reg::RDMPYL::address);
        u8 hi = hal::read8(reg::RDMPYH::address);
        *remainder = static_cast<u16>(lo | (hi << 8));
    }
}

// Integer square root using binary search
// Returns floor(sqrt(n))
u16 isqrt(u32 n) {
    if (n == 0) return 0;
    if (n == 1) return 1;

    // Initial estimate
    u32 x = n;
    u32 y = (x + 1) >> 1;

    // Newton's method iterations
    while (y < x) {
        x = y;
        y = (x + n / x) >> 1;
    }

    return static_cast<u16>(x);
}

} // namespace snes::math
