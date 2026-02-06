// OAM buffer definitions for SNES_TESTING mode
// In production builds, these are inline in the header

#ifdef SNES_TESTING

#include <snes/ppu.hpp>

namespace snes::ppu {

// OAM shadow buffers for testing
OAMEntry oam_low[128];
u8 oam_high[32];

} // namespace snes::ppu

#endif // SNES_TESTING
