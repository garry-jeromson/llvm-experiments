#pragma once

#include "types.hpp"

namespace snes::hal {

// Hardware abstraction interface for register access
// This allows mocking for unit tests while compiling to direct memory access in release builds
struct IRegisterAccess {
    virtual void write8(u32 addr, u8 val) = 0;
    virtual u8 read8(u32 addr) = 0;
    virtual void write16(u32 addr, u16 val) = 0;
    virtual u16 read16(u32 addr) = 0;
    virtual ~IRegisterAccess() = default;
};

#ifdef SNES_TESTING

// Testing mode: use virtual HAL that can be mocked
IRegisterAccess& get_hal();
void set_hal(IRegisterAccess& hal);

inline void write8(u32 addr, u8 val) { get_hal().write8(addr, val); }
inline u8 read8(u32 addr) { return get_hal().read8(addr); }
inline void write16(u32 addr, u16 val) { get_hal().write16(addr, val); }
inline u16 read16(u32 addr) { return get_hal().read16(addr); }

#else

// Production mode: direct memory access (zero overhead)
inline void write8(u32 addr, u8 val) {
    *reinterpret_cast<volatile u8*>(addr) = val;
}

inline u8 read8(u32 addr) {
    return *reinterpret_cast<volatile u8*>(addr);
}

inline void write16(u32 addr, u16 val) {
    *reinterpret_cast<volatile u16*>(addr) = val;
}

inline u16 read16(u32 addr) {
    return *reinterpret_cast<volatile u16*>(addr);
}

#endif

// Real hardware implementation (used as default in testing mode)
struct HardwareAccess : IRegisterAccess {
    void write8(u32 addr, u8 val) override {
        *reinterpret_cast<volatile u8*>(addr) = val;
    }

    u8 read8(u32 addr) override {
        return *reinterpret_cast<volatile u8*>(addr);
    }

    void write16(u32 addr, u16 val) override {
        *reinterpret_cast<volatile u16*>(addr) = val;
    }

    u16 read16(u32 addr) override {
        return *reinterpret_cast<volatile u16*>(addr);
    }
};

} // namespace snes::hal
