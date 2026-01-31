#include <snes/hal.hpp>

namespace snes::hal {

#ifdef SNES_TESTING

// Default to hardware access
static HardwareAccess default_hal;
static IRegisterAccess* current_hal = &default_hal;

IRegisterAccess& get_hal() {
    return *current_hal;
}

void set_hal(IRegisterAccess& hal) {
    current_hal = &hal;
}

#endif

} // namespace snes::hal
