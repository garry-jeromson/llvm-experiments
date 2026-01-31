#pragma once

// SNES SDK - Master Include Header
// Include this single header to get access to all SDK functionality

// Core types and hardware abstraction
#include "types.hpp"
#include "hal.hpp"
#include "registers.hpp"

// Modules
#include "input.hpp"
#include "dma.hpp"
#include "ppu.hpp"
#include "math.hpp"
#include "audio.hpp"
#include "superfx.hpp"

// Version information
namespace snes {
    constexpr int SDK_VERSION_MAJOR = 1;
    constexpr int SDK_VERSION_MINOR = 0;
    constexpr int SDK_VERSION_PATCH = 0;

    // Initialize all SDK subsystems
    inline void init() {
        input::init();
        // Other subsystems init as needed
    }
}
