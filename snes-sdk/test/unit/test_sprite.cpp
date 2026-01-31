// Unit tests for Sprite class
#ifndef SNES_TESTING
#define SNES_TESTING
#endif
#include "test_framework.hpp"
#include "fake_hal.hpp"
#include <snes/ppu.hpp>

using namespace snes;
using namespace snes::ppu;

// Helper to set up fake HAL
struct SpriteTestFixture {
    snes::testing::FakeRegisterAccess fake;

    SpriteTestFixture() {
        fake.clear();
        hal::set_hal(fake);

        // Clear OAM buffers
        for (int i = 0; i < 128; i++) {
            oam_low[i].x_low = 0;
            oam_low[i].y = 0;
            oam_low[i].tile = 0;
            oam_low[i].attr = 0;
        }
        for (int i = 0; i < 32; i++) {
            oam_high[i] = 0;
        }
    }
};

TEST(sprite_set_pos_basic) {
    SpriteTestFixture f;
    Sprite spr(0);

    spr.set_pos(100, 80);

    ASSERT_EQ(oam_low[0].x_low, 100);
    ASSERT_EQ(oam_low[0].y, 80);
}

TEST(sprite_set_pos_x_high_bit) {
    SpriteTestFixture f;
    Sprite spr(0);

    // X position > 255 sets high bit
    spr.set_pos(300, 80);  // 300 = 0x12C, low = 0x2C, high bit = 1

    ASSERT_EQ(oam_low[0].x_low, 0x2C);  // 300 & 0xFF = 44
    ASSERT_EQ(oam_low[0].y, 80);

    // High bit should be set (bit 0 of first byte in high table)
    ASSERT_EQ(oam_high[0] & 0x01, 0x01);
}

TEST(sprite_set_pos_negative_x) {
    SpriteTestFixture f;
    Sprite spr(0);

    // Negative X wraps around
    spr.set_pos(-16, 80);

    // -16 as signed = 0xFFF0 (or similar depending on representation)
    // Low 8 bits = 0xF0 = 240
    ASSERT_EQ(oam_low[0].x_low, 0xF0);
}

TEST(sprite_set_tile_basic) {
    SpriteTestFixture f;
    Sprite spr(0);

    spr.set_tile(42);

    ASSERT_EQ(oam_low[0].tile, 42);
    ASSERT_EQ(oam_low[0].attr & 0x01, 0);  // High bit clear
}

TEST(sprite_set_tile_high_bit) {
    SpriteTestFixture f;
    Sprite spr(0);

    // Tile > 255 sets bit 8 in attr
    spr.set_tile(300);  // 0x12C

    ASSERT_EQ(oam_low[0].tile, 0x2C);  // Low 8 bits
    ASSERT_EQ(oam_low[0].attr & 0x01, 0x01);  // Bit 8 in attr bit 0
}

TEST(sprite_set_tile_palette) {
    SpriteTestFixture f;
    Sprite spr(0);

    spr.set_tile(0, 3);  // Palette 3

    // Palette in bits 1-3
    ASSERT_EQ((oam_low[0].attr >> 1) & 0x07, 3);
}

TEST(sprite_set_tile_hflip) {
    SpriteTestFixture f;
    Sprite spr(0);

    spr.set_tile(0, 0, true, false);

    // H-flip is bit 6
    ASSERT_EQ(oam_low[0].attr & 0x40, 0x40);
    ASSERT_EQ(oam_low[0].attr & 0x80, 0x00);  // V-flip clear
}

TEST(sprite_set_tile_vflip) {
    SpriteTestFixture f;
    Sprite spr(0);

    spr.set_tile(0, 0, false, true);

    // V-flip is bit 7
    ASSERT_EQ(oam_low[0].attr & 0x80, 0x80);
    ASSERT_EQ(oam_low[0].attr & 0x40, 0x00);  // H-flip clear
}

TEST(sprite_set_priority) {
    SpriteTestFixture f;
    Sprite spr(0);

    // Set initial tile to have known attr
    spr.set_tile(0, 0);

    spr.set_priority(2);

    // Priority in bits 4-5
    ASSERT_EQ((oam_low[0].attr >> 4) & 0x03, 2);
}

TEST(sprite_set_size) {
    SpriteTestFixture f;
    Sprite spr(0);

    spr.set_size(true);  // Large size

    // Size bit is at position 1 in high table (after X high bit)
    // Sprite 0: byte 0, bits 0-1, so size is bit 1
    ASSERT_EQ(oam_high[0] & 0x02, 0x02);

    spr.set_size(false);  // Small size
    ASSERT_EQ(oam_high[0] & 0x02, 0x00);
}

TEST(sprite_hide) {
    SpriteTestFixture f;
    Sprite spr(0);

    spr.set_pos(100, 50);
    spr.hide();

    ASSERT_EQ(oam_low[0].y, 240);  // Off-screen
}

TEST(sprite_different_ids) {
    SpriteTestFixture f;
    Sprite spr5(5);
    Sprite spr10(10);

    spr5.set_pos(50, 50);
    spr5.set_tile(100);

    spr10.set_pos(150, 150);
    spr10.set_tile(200);

    ASSERT_EQ(oam_low[5].x_low, 50);
    ASSERT_EQ(oam_low[5].tile, 100);

    ASSERT_EQ(oam_low[10].x_low, 150);
    ASSERT_EQ(oam_low[10].tile, 200);
}

TEST(sprite_high_table_different_sprites) {
    SpriteTestFixture f;

    // Test sprites in different positions of high table
    // Each byte holds 4 sprites' high bits

    Sprite spr0(0);
    Sprite spr1(1);
    Sprite spr2(2);
    Sprite spr3(3);
    Sprite spr4(4);  // In byte 1

    // Set different X high bits
    spr0.set_pos(256, 0);  // Bit 0
    spr1.set_pos(256, 0);  // Bit 2
    spr2.set_pos(256, 0);  // Bit 4
    spr3.set_pos(256, 0);  // Bit 6
    spr4.set_pos(256, 0);  // Byte 1, bit 0

    // Check byte 0: all X high bits set (bits 0, 2, 4, 6)
    ASSERT_EQ(oam_high[0] & 0x55, 0x55);  // 0b01010101

    // Check byte 1: sprite 4's X high bit
    ASSERT_EQ(oam_high[1] & 0x01, 0x01);
}

TEST(sprites_clear_all) {
    SpriteTestFixture f;

    // Set some sprites
    Sprite spr0(0);
    Sprite spr1(1);
    spr0.set_pos(100, 100);
    spr1.set_pos(200, 200);

    sprites_clear();

    // All should be off-screen
    for (int i = 0; i < 128; i++) {
        ASSERT_EQ(oam_low[i].y, 240);
    }
}
