#include <snes/dma.hpp>
#include <cstdint>

namespace snes::dma {

namespace {
    // Helper to get DMA register base for channel
    constexpr u32 dma_base(Channel ch) {
        return 0x4300 + (static_cast<u8>(ch) * 0x10);
    }

    // Helper to set up DMA channel registers
    void setup_channel(Channel ch, u8 ctrl, u8 dest_reg, u32 src_addr, u16 size) {
        u32 base = dma_base(ch);

        // DMA control
        hal::write8(base + 0, ctrl);

        // B-bus destination
        hal::write8(base + 1, dest_reg);

        // A-bus source address (24-bit)
        hal::write8(base + 2, static_cast<u8>(src_addr & 0xFF));
        hal::write8(base + 3, static_cast<u8>((src_addr >> 8) & 0xFF));
        hal::write8(base + 4, static_cast<u8>((src_addr >> 16) & 0xFF));

        // Transfer size
        hal::write8(base + 5, static_cast<u8>(size & 0xFF));
        hal::write8(base + 6, static_cast<u8>(size >> 8));
    }
}

void to_vram(Channel ch, u16 vram_addr, const void* src, u16 size) {
    // Set VRAM address and increment mode
    hal::write8(reg::VMAIN::address, vmain::INC_HIGH | vmain::INC_1);
    hal::write8(reg::VMADDL::address, static_cast<u8>(vram_addr & 0xFF));
    hal::write8(reg::VMADDH::address, static_cast<u8>(vram_addr >> 8));

    // Set up DMA to VRAM (word transfer: low then high byte)
    u32 src_addr = static_cast<u32>(reinterpret_cast<std::uintptr_t>(src));
    setup_channel(ch, mode::WORD_TO_PAIR | mode::A_TO_B | mode::INC_ADDR,
                  dest::VRAM, src_addr, size);

    // Start DMA
    start(1 << static_cast<u8>(ch));
}

void to_cgram(Channel ch, u8 start_color, const void* src, u16 size) {
    // Set CGRAM address
    hal::write8(reg::CGADD::address, start_color);

    // Set up DMA to CGRAM (byte transfer, alternating low/high)
    u32 src_addr = static_cast<u32>(reinterpret_cast<std::uintptr_t>(src));
    setup_channel(ch, mode::BYTE_TO_SINGLE | mode::A_TO_B | mode::INC_ADDR,
                  dest::CGRAM, src_addr, size);

    // Start DMA
    start(1 << static_cast<u8>(ch));
}

void to_oam(Channel ch, const void* src, u16 size) {
    to_oam_at(ch, 0, src, size);
}

void to_oam_at(Channel ch, u16 oam_addr, const void* src, u16 size) {
    // Set OAM address
    hal::write8(reg::OAMADDL::address, static_cast<u8>(oam_addr & 0xFF));
    hal::write8(reg::OAMADDH::address, static_cast<u8>(oam_addr >> 8));

    // Set up DMA to OAM
    u32 src_addr = static_cast<u32>(reinterpret_cast<std::uintptr_t>(src));
    setup_channel(ch, mode::BYTE_TO_SINGLE | mode::A_TO_B | mode::INC_ADDR,
                  dest::OAM, src_addr, size);

    // Start DMA
    start(1 << static_cast<u8>(ch));
}

void transfer(Channel ch, u8 ctrl, u8 dest_reg, u32 src, u16 size) {
    setup_channel(ch, ctrl, dest_reg, src, size);
    start(1 << static_cast<u8>(ch));
}

void start(u8 channel_mask) {
    hal::write8(reg::MDMAEN::address, channel_mask);
}

void HdmaChannel::setup(u8 target_reg, const void* table, u8 transfer_mode) {
    u32 base = dma_base(static_cast<Channel>(m_channel));
    u32 table_addr = static_cast<u32>(reinterpret_cast<std::uintptr_t>(table));

    // HDMA control (direction = A to B)
    hal::write8(base + 0, transfer_mode | mode::A_TO_B);

    // B-bus destination
    hal::write8(base + 1, target_reg);

    // Table address (A-bus, 24-bit)
    hal::write8(base + 2, static_cast<u8>(table_addr & 0xFF));
    hal::write8(base + 3, static_cast<u8>((table_addr >> 8) & 0xFF));
    hal::write8(base + 4, static_cast<u8>((table_addr >> 16) & 0xFF));
}

void HdmaChannel::enable() {
    u8 current = hal::read8(reg::HDMAEN::address);
    hal::write8(reg::HDMAEN::address, current | (1 << m_channel));
}

void HdmaChannel::disable() {
    u8 current = hal::read8(reg::HDMAEN::address);
    hal::write8(reg::HDMAEN::address, current & ~(1 << m_channel));
}

void hdma_enable(u8 channel_mask) {
    u8 current = hal::read8(reg::HDMAEN::address);
    hal::write8(reg::HDMAEN::address, current | channel_mask);
}

void hdma_disable(u8 channel_mask) {
    u8 current = hal::read8(reg::HDMAEN::address);
    hal::write8(reg::HDMAEN::address, current & ~channel_mask);
}

void hdma_disable_all() {
    hal::write8(reg::HDMAEN::address, 0);
}

} // namespace snes::dma
