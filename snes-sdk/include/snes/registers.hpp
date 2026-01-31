#pragma once

#include "types.hpp"
#include "hal.hpp"

namespace snes {

// Type-safe write-only register
template<u32 Addr>
struct WReg {
    static void write(u8 val) {
        hal::write8(Addr, val);
    }

    static constexpr u32 address = Addr;
};

// Type-safe read-only register
template<u32 Addr>
struct RReg {
    static u8 read() {
        return hal::read8(Addr);
    }

    static constexpr u32 address = Addr;
};

// Type-safe read-write register
template<u32 Addr>
struct RWReg {
    static void write(u8 val) {
        hal::write8(Addr, val);
    }

    static u8 read() {
        return hal::read8(Addr);
    }

    static constexpr u32 address = Addr;
};

// 16-bit write-only register (low byte first, auto-increment)
template<u32 AddrLo>
struct WReg16 {
    static void write(u16 val) {
        hal::write8(AddrLo, static_cast<u8>(val & 0xFF));
        hal::write8(AddrLo + 1, static_cast<u8>(val >> 8));
    }

    static constexpr u32 address = AddrLo;
};

// 16-bit read-only register
template<u32 AddrLo>
struct RReg16 {
    static u16 read() {
        u8 lo = hal::read8(AddrLo);
        u8 hi = hal::read8(AddrLo + 1);
        return static_cast<u16>(lo | (hi << 8));
    }

    static constexpr u32 address = AddrLo;
};

// PPU Registers ($2100-$213F)
namespace reg {
    // Screen Display
    using INIDISP  = WReg<0x2100>;   // Display control (brightness, force blank)
    using OBSEL    = WReg<0x2101>;   // Object size and base
    using OAMADDL  = WReg<0x2102>;   // OAM address low
    using OAMADDH  = WReg<0x2103>;   // OAM address high
    using OAMDATA  = WReg<0x2104>;   // OAM data write

    // Background Mode and Character Size
    using BGMODE   = WReg<0x2105>;   // BG mode and character size
    using MOSAIC   = WReg<0x2106>;   // Mosaic size and enable

    // BG Tilemap Address
    using BG1SC    = WReg<0x2107>;   // BG1 tilemap address and size
    using BG2SC    = WReg<0x2108>;   // BG2 tilemap address and size
    using BG3SC    = WReg<0x2109>;   // BG3 tilemap address and size
    using BG4SC    = WReg<0x210A>;   // BG4 tilemap address and size

    // BG Character Data Address
    using BG12NBA  = WReg<0x210B>;   // BG1/BG2 character data address
    using BG34NBA  = WReg<0x210C>;   // BG3/BG4 character data address

    // BG Scroll
    using BG1HOFS  = WReg<0x210D>;   // BG1 horizontal scroll
    using BG1VOFS  = WReg<0x210E>;   // BG1 vertical scroll
    using BG2HOFS  = WReg<0x210F>;   // BG2 horizontal scroll
    using BG2VOFS  = WReg<0x2110>;   // BG2 vertical scroll
    using BG3HOFS  = WReg<0x2111>;   // BG3 horizontal scroll
    using BG3VOFS  = WReg<0x2112>;   // BG3 vertical scroll
    using BG4HOFS  = WReg<0x2113>;   // BG4 horizontal scroll
    using BG4VOFS  = WReg<0x2114>;   // BG4 vertical scroll

    // VRAM Access
    using VMAIN    = WReg<0x2115>;   // VRAM address increment mode
    using VMADDL   = WReg<0x2116>;   // VRAM address low
    using VMADDH   = WReg<0x2117>;   // VRAM address high
    using VMDATAL  = WReg<0x2118>;   // VRAM data write low
    using VMDATAH  = WReg<0x2119>;   // VRAM data write high
    using RDVRAML  = RReg<0x2139>;   // VRAM data read low
    using RDVRAMH  = RReg<0x213A>;   // VRAM data read high

    // Mode 7 Registers
    using M7SEL    = WReg<0x211A>;   // Mode 7 settings
    using M7A      = WReg<0x211B>;   // Mode 7 matrix A (cosine)
    using M7B      = WReg<0x211C>;   // Mode 7 matrix B (sine)
    using M7C      = WReg<0x211D>;   // Mode 7 matrix C (-sine)
    using M7D      = WReg<0x211E>;   // Mode 7 matrix D (cosine)
    using M7X      = WReg<0x211F>;   // Mode 7 center X
    using M7Y      = WReg<0x2120>;   // Mode 7 center Y

    // CGRAM (Palette) Access
    using CGADD    = WReg<0x2121>;   // CGRAM address
    using CGDATA   = WReg<0x2122>;   // CGRAM data write
    using RDCGRAM  = RReg<0x213B>;   // CGRAM data read

    // Window Mask Settings
    using W12SEL   = WReg<0x2123>;   // Window 1/2 mask settings for BG1/BG2
    using W34SEL   = WReg<0x2124>;   // Window 1/2 mask settings for BG3/BG4
    using WOBJSEL  = WReg<0x2125>;   // Window 1/2 mask settings for OBJ/Color
    using WH0      = WReg<0x2126>;   // Window 1 left position
    using WH1      = WReg<0x2127>;   // Window 1 right position
    using WH2      = WReg<0x2128>;   // Window 2 left position
    using WH3      = WReg<0x2129>;   // Window 2 right position
    using WBGLOG   = WReg<0x212A>;   // Window mask logic for BG
    using WOBJLOG  = WReg<0x212B>;   // Window mask logic for OBJ/Color

    // Main/Sub Screen Designation
    using TM       = WReg<0x212C>;   // Main screen designation
    using TS       = WReg<0x212D>;   // Sub screen designation
    using TMW      = WReg<0x212E>;   // Window mask main screen
    using TSW      = WReg<0x212F>;   // Window mask sub screen

    // Color Math
    using CGWSEL   = WReg<0x2130>;   // Color addition select
    using CGADSUB  = WReg<0x2131>;   // Color math designation
    using COLDATA  = WReg<0x2132>;   // Fixed color data

    // Screen Mode
    using SETINI   = WReg<0x2133>;   // Screen mode/video select

    // Multiply Result
    using MPYL     = RReg<0x2134>;   // Multiply result low
    using MPYM     = RReg<0x2135>;   // Multiply result mid
    using MPYH     = RReg<0x2136>;   // Multiply result high

    // PPU Status
    using SLHV     = RReg<0x2137>;   // Latch H/V counter
    using RDOAM    = RReg<0x2138>;   // OAM data read
    using OPHCT    = RReg<0x213C>;   // H counter
    using OPVCT    = RReg<0x213D>;   // V counter
    using STAT77   = RReg<0x213E>;   // PPU1 status
    using STAT78   = RReg<0x213F>;   // PPU2 status

    // CPU Registers ($4200-$421F)
    using NMITIMEN = WReg<0x4200>;   // NMI/IRQ enable
    using WRIO     = WReg<0x4201>;   // Programmable I/O port
    using WRMPYA   = WReg<0x4202>;   // Multiplicand A
    using WRMPYB   = WReg<0x4203>;   // Multiplicand B
    using WRDIVL   = WReg<0x4204>;   // Dividend low
    using WRDIVH   = WReg<0x4205>;   // Dividend high
    using WRDIVB   = WReg<0x4206>;   // Divisor
    using HTIMEL   = WReg<0x4207>;   // H-count timer low
    using HTIMEH   = WReg<0x4208>;   // H-count timer high
    using VTIMEL   = WReg<0x4209>;   // V-count timer low
    using VTIMEH   = WReg<0x420A>;   // V-count timer high
    using MDMAEN   = WReg<0x420B>;   // DMA enable
    using HDMAEN   = WReg<0x420C>;   // HDMA enable
    using MEMSEL   = WReg<0x420D>;   // ROM speed

    using RDNMI    = RReg<0x4210>;   // NMI flag and version
    using TIMEUP   = RReg<0x4211>;   // IRQ flag
    using HVBJOY   = RReg<0x4212>;   // H/V blank and joypad status
    using RDIO     = RReg<0x4213>;   // Programmable I/O port read

    // Hardware multiply/divide results
    using RDDIVL   = RReg<0x4214>;   // Quotient low
    using RDDIVH   = RReg<0x4215>;   // Quotient high
    using RDMPYL   = RReg<0x4216>;   // Product/remainder low
    using RDMPYH   = RReg<0x4217>;   // Product/remainder high

    // Joypad Registers
    using JOY1L    = RReg<0x4218>;   // Joypad 1 low
    using JOY1H    = RReg<0x4219>;   // Joypad 1 high
    using JOY2L    = RReg<0x421A>;   // Joypad 2 low
    using JOY2H    = RReg<0x421B>;   // Joypad 2 high
    using JOY3L    = RReg<0x421C>;   // Joypad 3 low
    using JOY3H    = RReg<0x421D>;   // Joypad 3 high
    using JOY4L    = RReg<0x421E>;   // Joypad 4 low
    using JOY4H    = RReg<0x421F>;   // Joypad 4 high

    // DMA Registers ($4300-$437F, channels 0-7)
    // Channel n at $43n0-$43nF
    template<u8 Ch>
    struct DMA {
        static_assert(Ch < 8, "DMA channel must be 0-7");
        static constexpr u32 base = 0x4300 + (Ch * 0x10);

        using CTRL   = WReg<base + 0>;   // DMA control
        using DEST   = WReg<base + 1>;   // DMA destination (B-bus address)
        using SRCL   = WReg<base + 2>;   // Source address low
        using SRCM   = WReg<base + 3>;   // Source address mid
        using SRCH   = WReg<base + 4>;   // Source address bank
        using SIZEL  = WReg<base + 5>;   // Transfer size low
        using SIZEH  = WReg<base + 6>;   // Transfer size high
        using HDMA   = WReg<base + 7>;   // HDMA indirect bank
        using ADDRL  = WReg<base + 8>;   // HDMA table address low
        using ADDRH  = WReg<base + 9>;   // HDMA table address high
        using LINES  = WReg<base + 10>;  // HDMA line counter
    };

    // APU Communication Ports
    using APUIO0   = RWReg<0x2140>;  // APU I/O port 0
    using APUIO1   = RWReg<0x2141>;  // APU I/O port 1
    using APUIO2   = RWReg<0x2142>;  // APU I/O port 2
    using APUIO3   = RWReg<0x2143>;  // APU I/O port 3
}

// VMAIN increment modes
namespace vmain {
    constexpr u8 INC_1     = 0x00;  // Increment by 1
    constexpr u8 INC_32    = 0x01;  // Increment by 32
    constexpr u8 INC_128   = 0x02;  // Increment by 128
    constexpr u8 INC_LOW   = 0x00;  // Increment on low byte write
    constexpr u8 INC_HIGH  = 0x80;  // Increment on high byte write
}

// BGMODE values
namespace bgmode {
    constexpr u8 MODE_0    = 0x00;  // 4 BGs, 2bpp each
    constexpr u8 MODE_1    = 0x01;  // 2 BGs 4bpp, 1 BG 2bpp
    constexpr u8 MODE_2    = 0x02;  // 2 BGs 4bpp, offset-per-tile
    constexpr u8 MODE_3    = 0x03;  // 1 BG 8bpp, 1 BG 4bpp
    constexpr u8 MODE_4    = 0x04;  // 1 BG 8bpp, 1 BG 2bpp, offset-per-tile
    constexpr u8 MODE_5    = 0x05;  // 1 BG 4bpp, 1 BG 2bpp, hi-res
    constexpr u8 MODE_6    = 0x06;  // 1 BG 4bpp, offset-per-tile, hi-res
    constexpr u8 MODE_7    = 0x07;  // 1 BG 8bpp, rotation/scaling

    constexpr u8 BG3_PRIO  = 0x08;  // BG3 priority (Mode 1 only)
    constexpr u8 BG1_16X16 = 0x10;  // BG1 16x16 tiles
    constexpr u8 BG2_16X16 = 0x20;  // BG2 16x16 tiles
    constexpr u8 BG3_16X16 = 0x40;  // BG3 16x16 tiles
    constexpr u8 BG4_16X16 = 0x80;  // BG4 16x16 tiles
}

// TM/TS screen designation bits
namespace screen {
    constexpr u8 BG1 = 0x01;
    constexpr u8 BG2 = 0x02;
    constexpr u8 BG3 = 0x04;
    constexpr u8 BG4 = 0x08;
    constexpr u8 OBJ = 0x10;
}

// NMITIMEN flags
namespace nmi {
    constexpr u8 JOYPAD_ENABLE = 0x01;  // Enable joypad auto-read
    constexpr u8 HIRQ_ENABLE   = 0x10;  // Enable H-IRQ
    constexpr u8 VIRQ_ENABLE   = 0x20;  // Enable V-IRQ
    constexpr u8 NMI_ENABLE    = 0x80;  // Enable NMI on V-blank
}

} // namespace snes
