// Unit tests for Joypad class
#ifndef SNES_TESTING
#define SNES_TESTING
#endif
#include "test_framework.hpp"
#include "fake_hal.hpp"
#include <snes/input.hpp>

using namespace snes;
using namespace snes::input;

// Helper to set up fake HAL before each test
struct JoypadTestFixture {
    snes::testing::FakeRegisterAccess fake;

    JoypadTestFixture() {
        fake.clear();
        hal::set_hal(fake);
    }

    void set_joypad_state(u8 id, u16 buttons) {
        u32 addr = 0x4218 + (id * 2);
        fake.set_read_value(static_cast<u32>(addr), static_cast<u8>(buttons & 0xFF));
        fake.set_read_value(static_cast<u32>(addr + 1), static_cast<u8>(buttons >> 8));
    }
};

TEST(joypad_initial_state) {
    JoypadTestFixture f;
    Joypad pad(0);

    ASSERT_EQ(pad.raw(), 0);
    ASSERT_FALSE(pad.held(Button::A));
    ASSERT_FALSE(pad.pressed(Button::A));
}

TEST(joypad_update_reads_buttons) {
    JoypadTestFixture f;
    Joypad pad(0);

    // Set A button pressed (0x0080)
    f.set_joypad_state(0, 0x0080);

    pad.update();

    ASSERT_TRUE(pad.held(Button::A));
    ASSERT_EQ(pad.raw(), 0x0080);
}

TEST(joypad_pressed_detection) {
    JoypadTestFixture f;
    Joypad pad(0);

    // Frame 1: no buttons
    f.set_joypad_state(0, 0x0000);
    pad.update();
    ASSERT_FALSE(pad.pressed(Button::A));
    ASSERT_FALSE(pad.held(Button::A));

    // Frame 2: A pressed
    f.set_joypad_state(0, 0x0080);
    pad.update();
    ASSERT_TRUE(pad.pressed(Button::A));  // Just pressed
    ASSERT_TRUE(pad.held(Button::A));

    // Frame 3: A still held
    pad.update();
    ASSERT_FALSE(pad.pressed(Button::A));  // Not newly pressed
    ASSERT_TRUE(pad.held(Button::A));      // Still held
}

TEST(joypad_released_detection) {
    JoypadTestFixture f;
    Joypad pad(0);

    // Frame 1: A pressed
    f.set_joypad_state(0, 0x0080);
    pad.update();
    ASSERT_FALSE(pad.released(Button::A));

    // Frame 2: A released
    f.set_joypad_state(0, 0x0000);
    pad.update();
    ASSERT_TRUE(pad.released(Button::A));
    ASSERT_FALSE(pad.held(Button::A));

    // Frame 3: still released
    pad.update();
    ASSERT_FALSE(pad.released(Button::A));  // Not newly released
}

TEST(joypad_multiple_buttons) {
    JoypadTestFixture f;
    Joypad pad(0);

    // A (0x0080) + B (0x8000) = 0x8080
    f.set_joypad_state(0, 0x8080);
    pad.update();

    ASSERT_TRUE(pad.held(Button::A));
    ASSERT_TRUE(pad.held(Button::B));
    ASSERT_FALSE(pad.held(Button::X));
}

TEST(joypad_held_all) {
    JoypadTestFixture f;
    Joypad pad(0);

    // A + B
    f.set_joypad_state(0, static_cast<u16>(Button::A) | static_cast<u16>(Button::B));
    pad.update();

    ASSERT_TRUE(pad.held_all(static_cast<u16>(Button::A) | static_cast<u16>(Button::B)));
    ASSERT_FALSE(pad.held_all(static_cast<u16>(Button::A) | static_cast<u16>(Button::X)));
}

TEST(joypad_held_any) {
    JoypadTestFixture f;
    Joypad pad(0);

    // Just A pressed
    f.set_joypad_state(0, static_cast<u16>(Button::A));
    pad.update();

    ASSERT_TRUE(pad.held_any(static_cast<u16>(Button::A) | static_cast<u16>(Button::B)));
    ASSERT_FALSE(pad.held_any(static_cast<u16>(Button::X) | static_cast<u16>(Button::Y)));
}

TEST(joypad_direction_cardinals) {
    JoypadTestFixture f;
    Joypad pad(0);

    f.set_joypad_state(0, static_cast<u16>(Button::Up));
    pad.update();
    ASSERT_EQ(static_cast<int>(pad.direction()), static_cast<int>(Direction::Up));

    f.set_joypad_state(0, static_cast<u16>(Button::Down));
    pad.update();
    ASSERT_EQ(static_cast<int>(pad.direction()), static_cast<int>(Direction::Down));

    f.set_joypad_state(0, static_cast<u16>(Button::Left));
    pad.update();
    ASSERT_EQ(static_cast<int>(pad.direction()), static_cast<int>(Direction::Left));

    f.set_joypad_state(0, static_cast<u16>(Button::Right));
    pad.update();
    ASSERT_EQ(static_cast<int>(pad.direction()), static_cast<int>(Direction::Right));
}

TEST(joypad_direction_diagonals) {
    JoypadTestFixture f;
    Joypad pad(0);

    f.set_joypad_state(0, static_cast<u16>(Button::Up) | static_cast<u16>(Button::Left));
    pad.update();
    ASSERT_EQ(static_cast<int>(pad.direction()), static_cast<int>(Direction::UpLeft));

    f.set_joypad_state(0, static_cast<u16>(Button::Down) | static_cast<u16>(Button::Right));
    pad.update();
    ASSERT_EQ(static_cast<int>(pad.direction()), static_cast<int>(Direction::DownRight));
}

TEST(joypad_direction_none) {
    JoypadTestFixture f;
    Joypad pad(0);

    f.set_joypad_state(0, 0);
    pad.update();
    ASSERT_EQ(static_cast<int>(pad.direction()), static_cast<int>(Direction::None));
}

TEST(joypad_axis_x) {
    JoypadTestFixture f;
    Joypad pad(0);

    f.set_joypad_state(0, static_cast<u16>(Button::Left));
    pad.update();
    ASSERT_EQ(pad.axis_x(), -1);

    f.set_joypad_state(0, static_cast<u16>(Button::Right));
    pad.update();
    ASSERT_EQ(pad.axis_x(), 1);

    f.set_joypad_state(0, 0);
    pad.update();
    ASSERT_EQ(pad.axis_x(), 0);
}

TEST(joypad_axis_y) {
    JoypadTestFixture f;
    Joypad pad(0);

    f.set_joypad_state(0, static_cast<u16>(Button::Up));
    pad.update();
    ASSERT_EQ(pad.axis_y(), -1);

    f.set_joypad_state(0, static_cast<u16>(Button::Down));
    pad.update();
    ASSERT_EQ(pad.axis_y(), 1);

    f.set_joypad_state(0, 0);
    pad.update();
    ASSERT_EQ(pad.axis_y(), 0);
}

TEST(joypad_different_ids) {
    JoypadTestFixture f;
    Joypad pad0(0);
    Joypad pad1(1);

    // Different buttons on different pads
    f.set_joypad_state(0, static_cast<u16>(Button::A));
    f.set_joypad_state(1, static_cast<u16>(Button::B));

    pad0.update();
    pad1.update();

    ASSERT_TRUE(pad0.held(Button::A));
    ASSERT_FALSE(pad0.held(Button::B));
    ASSERT_FALSE(pad1.held(Button::A));
    ASSERT_TRUE(pad1.held(Button::B));
}
