#include <snes/superfx.hpp>
#include <snes/hal.hpp>
#include <snes/dma.hpp>

namespace snes::superfx {

// GSU register base
constexpr u32 GSU_BASE = 0x3000;

// General purpose registers (R0-R15) at $3000-$301F (16-bit each)
constexpr u32 GSU_R0 = 0x3000;

void init() {
    // Stop GSU if running
    stop();

    // Clear status
    hal::write8(reg::SFR, 0);
    hal::write8(reg::SFR + 1, 0);

    // Set default RAM bank
    hal::write8(reg::RAMBR, 0);

    // Set default screen mode (4bpp, 128 high)
    hal::write8(reg::SCMR, scmr::MODE_4BPP | scmr::HEIGHT_128);

    // Set default plot options
    hal::write8(reg::POR, 0);
}

bool detect() {
    // Try to write and read back a register
    // On non-SuperFX carts, this will likely return open bus or 0xFF

    // Save current value
    u8 old = hal::read8(reg::SCMR);

    // Write test pattern
    hal::write8(reg::SCMR, 0x55);
    u8 test1 = hal::read8(reg::SCMR);

    hal::write8(reg::SCMR, 0xAA);
    u8 test2 = hal::read8(reg::SCMR);

    // Restore
    hal::write8(reg::SCMR, old);

    // Check if writes were reflected
    return (test1 == 0x55 || test1 == (0x55 & 0x3F)) &&
           (test2 == 0xAA || test2 == (0xAA & 0x3F));
}

void upload(u16 addr, const void* data, u16 size) {
    const u8* bytes = static_cast<const u8*>(data);

    // GSU RAM is typically at $6000-$7FFF in bank $00 or $70-$71
    // For simplicity, we assume GSU RAM is accessible via direct writes

    // Stop GSU before uploading
    stop();

    // Upload byte by byte
    // In practice, you'd use DMA for larger transfers
    for (u16 i = 0; i < size; i++) {
        hal::write8(0x6000 + addr + i, bytes[i]);
    }
}

void run(u16 addr) {
    // Set program counter (R15) to start address
    set_reg(15, addr);

    // Set GO flag in SFR
    u16 sfr = get_status();
    sfr |= sfr::GO;
    hal::write8(reg::SFR, static_cast<u8>(sfr & 0xFF));
    hal::write8(reg::SFR + 1, static_cast<u8>(sfr >> 8));
}

void wait() {
    while (busy()) {
        // Spin
    }
}

bool busy() {
    return (get_status() & sfr::GO) != 0;
}

void stop() {
    // Clear GO flag
    u16 sfr = get_status();
    sfr &= ~sfr::GO;
    hal::write8(reg::SFR, static_cast<u8>(sfr & 0xFF));
    hal::write8(reg::SFR + 1, static_cast<u8>(sfr >> 8));
}

void set_reg(u8 reg_num, u16 value) {
    if (reg_num > 15) return;

    u32 addr = GSU_R0 + (reg_num * 2);
    hal::write8(addr, static_cast<u8>(value & 0xFF));
    hal::write8(addr + 1, static_cast<u8>(value >> 8));
}

u16 get_reg(u8 reg_num) {
    if (reg_num > 15) return 0;

    u32 addr = GSU_R0 + (reg_num * 2);
    u8 lo = hal::read8(addr);
    u8 hi = hal::read8(addr + 1);
    return static_cast<u16>(lo | (hi << 8));
}

void set_rom_bank(u8 bank) {
    hal::write8(reg::ROMBR, bank);
}

void set_ram_bank(u8 bank) {
    hal::write8(reg::RAMBR, bank);
}

void set_screen_base(u8 page) {
    hal::write8(reg::SCBR, page);
}

void set_screen_mode(u8 bpp, u8 height_mode) {
    u8 mode = (bpp & 0x03) | (height_mode & 0x0C);
    hal::write8(reg::SCMR, mode);
}

void set_plot_options(u8 options) {
    hal::write8(reg::POR, options);
}

void enable_irq(bool enable) {
    // CFGR bit 7 controls IRQ enable
    u8 cfgr = hal::read8(reg::CFGR);
    if (enable) {
        cfgr |= 0x80;
    } else {
        cfgr &= ~0x80;
    }
    hal::write8(reg::CFGR, cfgr);
}

void clear_irq() {
    // Reading SFR clears the IRQ flag
    get_status();
}

u16 get_status() {
    u8 lo = hal::read8(reg::SFR);
    u8 hi = hal::read8(reg::SFR + 1);
    return static_cast<u16>(lo | (hi << 8));
}

void set_cache_base(u16 addr) {
    hal::write8(reg::CBR, static_cast<u8>(addr & 0xFF));
    hal::write8(reg::CBR + 1, static_cast<u8>(addr >> 8));
}

void flush_cache() {
    // Writing to CFGR bit 0 flushes cache
    u8 cfgr = hal::read8(reg::CFGR);
    hal::write8(reg::CFGR, cfgr | 0x01);
    hal::write8(reg::CFGR, cfgr & ~0x01);
}

// High-level graphics operations
// These would require uploading GSU code that implements the operations
// For now, these are placeholders

void clear_screen(u8 color) {
    // Would upload and run GSU clear routine
    (void)color;
}

void fill_rect(i16 x, i16 y, u16 width, u16 height, u8 color) {
    // Would upload and run GSU rect fill routine
    (void)x; (void)y; (void)width; (void)height; (void)color;
}

void draw_line(i16 x1, i16 y1, i16 x2, i16 y2, u8 color) {
    // Would upload and run GSU line routine
    (void)x1; (void)y1; (void)x2; (void)y2; (void)color;
}

void plot(i16 x, i16 y, u8 color) {
    // Would upload and run GSU plot routine
    // Or set up registers and use PLOT instruction
    (void)x; (void)y; (void)color;
}

void copy_to_vram(u16 vram_addr, u16 gsu_addr, u16 size) {
    // DMA from GSU RAM to VRAM
    // GSU RAM is at $70:0000 or $00:6000 depending on mapping

    // This is simplified - actual implementation needs proper bank handling
    dma::to_vram(dma::Channel::Ch0, vram_addr,
                 reinterpret_cast<const void*>(0x700000 + gsu_addr), size);
}

} // namespace snes::superfx
