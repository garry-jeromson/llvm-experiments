#include <snes/ppu.hpp>
#include <snes/dma.hpp>

namespace snes::ppu {

// Shadow OAM buffer
OAMEntry oam_low[128];
u8 oam_high[32];

// Text output state
static u16 text_tilemap_addr = 0x1000;  // Default tilemap address
static u8 text_palette = 0;

// Screen control

void screen_on(u8 brightness) {
    hal::write8(reg::INIDISP::address, brightness & 0x0F);
}

void screen_off() {
    hal::write8(reg::INIDISP::address, 0x80);  // Force blank
}

void wait_vblank() {
    // Wait for vblank to start
    while (!(hal::read8(reg::HVBJOY::address) & 0x80)) {
        // Spin
    }
}

bool in_vblank() {
    return (hal::read8(reg::HVBJOY::address) & 0x80) != 0;
}

void set_bgcolor(u8 r, u8 g, u8 b) {
    set_bgcolor(Color::from_rgb(r, g, b));
}

void set_bgcolor(Color color) {
    hal::write8(reg::CGADD::address, 0);  // Color index 0
    hal::write8(reg::CGDATA::address, static_cast<u8>(color.raw & 0xFF));
    hal::write8(reg::CGDATA::address, static_cast<u8>(color.raw >> 8));
}

void set_mode(u8 mode) {
    hal::write8(reg::BGMODE::address, mode & 0x07);
}

// Background implementation

void Background::set_tilemap(u16 vram_addr, u8 size) {
    // Tilemap address is in bits 2-7 (shifted left 9 for word address)
    // Size is in bits 0-1
    u8 val = static_cast<u8>(((vram_addr >> 9) << 2) | (size & 0x03));

    // Write to appropriate BGnSC register
    switch (m_id) {
        case 0: hal::write8(reg::BG1SC::address, val); break;
        case 1: hal::write8(reg::BG2SC::address, val); break;
        case 2: hal::write8(reg::BG3SC::address, val); break;
        case 3: hal::write8(reg::BG4SC::address, val); break;
    }
}

void Background::set_tiles(u16 vram_addr, bool bpp8) {
    // Character data address is in 4K increments
    u8 addr_val = static_cast<u8>((vram_addr >> 12) & 0x0F);

    // BG1/2 share one register, BG3/4 share another
    if (m_id < 2) {
        u8 current = 0;  // Would need to read, but PPU registers are write-only
        if (m_id == 0) {
            // BG1 in low nibble
            hal::write8(reg::BG12NBA::address, addr_val);
        } else {
            // BG2 in high nibble
            hal::write8(reg::BG12NBA::address, static_cast<u8>(addr_val << 4));
        }
    } else {
        if (m_id == 2) {
            hal::write8(reg::BG34NBA::address, addr_val);
        } else {
            hal::write8(reg::BG34NBA::address, static_cast<u8>(addr_val << 4));
        }
    }

    (void)bpp8;  // Mode determines bpp, not this register
}

void Background::scroll(i16 x, i16 y) {
    // Scroll registers are write-twice (low byte, then high byte)
    u32 hofs_addr, vofs_addr;

    switch (m_id) {
        case 0: hofs_addr = reg::BG1HOFS::address; vofs_addr = reg::BG1VOFS::address; break;
        case 1: hofs_addr = reg::BG2HOFS::address; vofs_addr = reg::BG2VOFS::address; break;
        case 2: hofs_addr = reg::BG3HOFS::address; vofs_addr = reg::BG3VOFS::address; break;
        case 3: hofs_addr = reg::BG4HOFS::address; vofs_addr = reg::BG4VOFS::address; break;
        default: return;
    }

    // Write horizontal scroll (write low byte, then high byte)
    hal::write8(hofs_addr, static_cast<u8>(x & 0xFF));
    hal::write8(hofs_addr, static_cast<u8>((x >> 8) & 0x03));

    // Write vertical scroll
    hal::write8(vofs_addr, static_cast<u8>(y & 0xFF));
    hal::write8(vofs_addr, static_cast<u8>((y >> 8) & 0x03));
}

void Background::enable() {
    u8 mask = 1 << m_id;
    u8 current = 0;  // Can't read TM, would need shadow
    hal::write8(reg::TM::address, current | mask);
}

void Background::disable() {
    u8 mask = static_cast<u8>(~(1 << m_id));
    u8 current = 0;  // Can't read TM
    hal::write8(reg::TM::address, current & mask);
}

void Background::enable_sub() {
    u8 mask = 1 << m_id;
    hal::write8(reg::TS::address, mask);
}

void Background::disable_sub() {
    hal::write8(reg::TS::address, 0);
}

void Background::set_tile_size(bool large) {
    // This modifies BGMODE which affects all BGs
    // In practice, you'd read the shadow value and modify it
    u8 mask = static_cast<u8>(0x10 << m_id);
    if (large) {
        hal::write8(reg::BGMODE::address, mask);
    }
}

// Sprite implementation

void Sprite::set_pos(i16 x, i16 y) {
    // X position: low 8 bits in OAM low table, bit 8 in high table
    oam_low[m_id].x_low = static_cast<u8>(x & 0xFF);
    oam_low[m_id].y = static_cast<u8>(y);

    // Update high table (bit 0 = x bit 8, bit 1 = size)
    u8 byte_idx = m_id >> 2;      // Which byte (0-31)
    u8 bit_pos = (m_id & 0x03) * 2;  // Which bit pair (0, 2, 4, 6)

    u8 x_high = (x >> 8) & 0x01;
    u8 mask = static_cast<u8>(~(0x01 << bit_pos));
    oam_high[byte_idx] = static_cast<u8>((oam_high[byte_idx] & mask) | (x_high << bit_pos));
}

void Sprite::set_tile(u16 tile, u8 palette, bool hflip, bool vflip) {
    // Tile low 8 bits
    oam_low[m_id].tile = static_cast<u8>(tile & 0xFF);

    // Attributes: vhoopppc
    // v = vflip, h = hflip, oo = priority, ppp = palette, c = tile bit 8
    u8 attr = static_cast<u8>(
        ((tile >> 8) & 0x01) |        // Tile bit 8
        ((palette & 0x07) << 1) |     // Palette
        (hflip ? 0x40 : 0) |          // H-flip
        (vflip ? 0x80 : 0)            // V-flip
    );
    oam_low[m_id].attr = attr;
}

void Sprite::set_priority(u8 prio) {
    oam_low[m_id].attr = static_cast<u8>((oam_low[m_id].attr & 0xCF) | ((prio & 0x03) << 4));
}

void Sprite::set_size(bool large) {
    u8 byte_idx = m_id >> 2;
    u8 bit_pos = ((m_id & 0x03) * 2) + 1;  // Size bit is after X high bit

    u8 mask = static_cast<u8>(~(0x01 << bit_pos));
    oam_high[byte_idx] = static_cast<u8>((oam_high[byte_idx] & mask) | ((large ? 1 : 0) << bit_pos));
}

void Sprite::hide() {
    // Move sprite off-screen (Y = 240 or higher wraps to top)
    oam_low[m_id].y = 240;
}

void sprites_update() {
    // Use DMA to upload OAM
    // First, set OAM address to 0
    hal::write8(reg::OAMADDL::address, 0);
    hal::write8(reg::OAMADDH::address, 0);

    // DMA low table (512 bytes)
    dma::to_oam(dma::Channel::Ch0, oam_low, sizeof(oam_low));

    // DMA high table (32 bytes)
    dma::to_oam_at(dma::Channel::Ch0, 0x0200, oam_high, sizeof(oam_high));
}

void sprites_clear() {
    // Set all sprites off-screen
    for (int i = 0; i < 128; i++) {
        oam_low[i].x_low = 0;
        oam_low[i].y = 240;  // Off-screen
        oam_low[i].tile = 0;
        oam_low[i].attr = 0;
    }

    // Clear high table (all small size, x bit 8 = 0)
    for (int i = 0; i < 32; i++) {
        oam_high[i] = 0;
    }
}

void sprites_enable() {
    // Enable OBJ on main screen
    hal::write8(reg::TM::address, screen::OBJ);
}

void sprites_disable() {
    hal::write8(reg::TM::address, 0);
}

void sprites_set_base(u16 base, u8 size_select) {
    // OBSEL: sssnnbbb
    // sss = size select, nn = name select, bbb = base address
    u8 val = static_cast<u8>(((base >> 13) & 0x07) | ((size_select & 0x07) << 5));
    hal::write8(reg::OBSEL::address, val);
}

// Mode 7 implementation

namespace mode7 {

void init() {
    // Set mode 7
    hal::write8(reg::BGMODE::address, bgmode::MODE_7);

    // Enable BG1 on main screen
    hal::write8(reg::TM::address, screen::BG1);

    // Default matrix (identity)
    set_matrix(256, 0, 0, 256);  // 1.0 in 8.8 fixed point
    set_center(0, 0);
    set_scroll(0, 0);
}

void set_matrix(i16 a, i16 b, i16 c, i16 d) {
    // Each register is written twice (low byte, high byte)
    hal::write8(reg::M7A::address, static_cast<u8>(a & 0xFF));
    hal::write8(reg::M7A::address, static_cast<u8>(a >> 8));

    hal::write8(reg::M7B::address, static_cast<u8>(b & 0xFF));
    hal::write8(reg::M7B::address, static_cast<u8>(b >> 8));

    hal::write8(reg::M7C::address, static_cast<u8>(c & 0xFF));
    hal::write8(reg::M7C::address, static_cast<u8>(c >> 8));

    hal::write8(reg::M7D::address, static_cast<u8>(d & 0xFF));
    hal::write8(reg::M7D::address, static_cast<u8>(d >> 8));
}

void set_center(i16 x, i16 y) {
    hal::write8(reg::M7X::address, static_cast<u8>(x & 0xFF));
    hal::write8(reg::M7X::address, static_cast<u8>(x >> 8));

    hal::write8(reg::M7Y::address, static_cast<u8>(y & 0xFF));
    hal::write8(reg::M7Y::address, static_cast<u8>(y >> 8));
}

void set_scroll(i16 x, i16 y) {
    // Mode 7 uses BG1 scroll registers
    hal::write8(reg::BG1HOFS::address, static_cast<u8>(x & 0xFF));
    hal::write8(reg::BG1HOFS::address, static_cast<u8>(x >> 8));

    hal::write8(reg::BG1VOFS::address, static_cast<u8>(y & 0xFF));
    hal::write8(reg::BG1VOFS::address, static_cast<u8>(y >> 8));
}

void set_rotation(u8 angle, Fixed8 scale) {
    // Would need sin/cos tables - implemented in math module
    // For now, just set identity scaled
    i16 s = scale.raw;
    set_matrix(s, 0, 0, s);
}

void set_flags(bool flip_x, bool flip_y, bool wrap) {
    u8 val = 0;
    if (flip_x) val |= 0x01;
    if (flip_y) val |= 0x02;
    if (!wrap) val |= 0x80;  // Bit 7 = 0 for wrap, 1 for transparent
    hal::write8(reg::M7SEL::address, val);
}

} // namespace mode7

// Text output implementation

void put_char(u16 x, u16 y, char c) {
    // Calculate tilemap position (32 tiles per row)
    u16 tile_pos = text_tilemap_addr + y * 32 + x;

    // Set VRAM address
    hal::write8(reg::VMAIN::address, vmain::INC_HIGH | vmain::INC_1);
    hal::write8(reg::VMADDL::address, static_cast<u8>(tile_pos & 0xFF));
    hal::write8(reg::VMADDH::address, static_cast<u8>(tile_pos >> 8));

    // Write tile number (ASCII value) and attribute
    hal::write8(reg::VMDATAL::address, static_cast<u8>(c));
    hal::write8(reg::VMDATAH::address, text_palette << 2);  // Palette in bits 2-4
}

void put_text(u16 x, u16 y, const char* str) {
    while (*str) {
        put_char(x++, y, *str++);
        if (x >= 32) {
            x = 0;
            y++;
        }
    }
}

void put_number(u16 x, u16 y, u16 num) {
    char buf[6];  // Max 65535 = 5 digits + null
    int i = 5;
    buf[i] = '\0';

    if (num == 0) {
        put_char(x, y, '0');
        return;
    }

    while (num > 0 && i > 0) {
        buf[--i] = static_cast<char>('0' + (num % 10));
        num /= 10;
    }

    put_text(x, y, &buf[i]);
}

void put_hex(u16 x, u16 y, u16 num) {
    static const char hex_chars[] = "0123456789ABCDEF";
    char buf[5];
    buf[0] = hex_chars[(num >> 12) & 0xF];
    buf[1] = hex_chars[(num >> 8) & 0xF];
    buf[2] = hex_chars[(num >> 4) & 0xF];
    buf[3] = hex_chars[num & 0xF];
    buf[4] = '\0';
    put_text(x, y, buf);
}

void clear_text() {
    // Fill tilemap with spaces
    hal::write8(reg::VMAIN::address, vmain::INC_HIGH | vmain::INC_1);
    hal::write8(reg::VMADDL::address, static_cast<u8>(text_tilemap_addr & 0xFF));
    hal::write8(reg::VMADDH::address, static_cast<u8>(text_tilemap_addr >> 8));

    // 32x32 = 1024 tiles
    for (int i = 0; i < 1024; i++) {
        hal::write8(reg::VMDATAL::address, ' ');
        hal::write8(reg::VMDATAH::address, 0);
    }
}

void set_text_palette(u8 palette) {
    text_palette = palette & 0x07;
}

void set_text_tilemap(u16 vram_addr) {
    text_tilemap_addr = vram_addr;
}

void upload_font(u16 vram_addr) {
    // Default font would be defined in font.s
    // This is a placeholder - actual implementation would DMA font data
    (void)vram_addr;
}

} // namespace snes::ppu
