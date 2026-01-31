// SNES SDK Unit Test Runner
// Compiles and runs on the host machine (not W65816)
//
// Build with:
//   clang++ -std=c++17 -I../../include -DSNES_TESTING run_unit_tests.cpp ../../src/hal.cpp -o run_tests

#ifndef SNES_TESTING
#define SNES_TESTING
#endif

#include "test_framework.hpp"

// Include test files (they define TEST() macros that auto-register)
#include "test_fixed8.cpp"
#include "test_math.cpp"

// Tests that need fake HAL also need the HAL implementation
#include <snes/hal.hpp>
#include "fake_hal.hpp"
#include "test_joypad.cpp"
#include "test_sprite.cpp"

int main() {
    std::printf("SNES SDK Unit Tests\n");
    std::printf("===================\n\n");
    return test::run_all_tests();
}
