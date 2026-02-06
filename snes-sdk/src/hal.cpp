// HAL implementation for SNES_TESTING mode
// Provides get_hal() and set_hal() to allow mocking hardware access in tests

#ifdef SNES_TESTING

#include <snes/hal.hpp>

namespace snes::hal {

// Default hardware access (never used in tests, but needed for initialization)
static HardwareAccess default_hal;

// Pointer to the current HAL implementation
static IRegisterAccess* current_hal = &default_hal;

// Get the current HAL implementation
IRegisterAccess& get_hal() {
    return *current_hal;
}

// Set the HAL implementation (for testing)
void set_hal(IRegisterAccess& hal) {
    current_hal = &hal;
}

} // namespace snes::hal

#endif // SNES_TESTING
