#include <snes/audio.hpp>
#include <snes/hal.hpp>
#include <snes/registers.hpp>

namespace snes::audio {

// APU port base addresses
constexpr u32 APU_PORT0 = 0x2140;
constexpr u32 APU_PORT1 = 0x2141;
constexpr u32 APU_PORT2 = 0x2142;
constexpr u32 APU_PORT3 = 0x2143;

// State tracking
static bool driver_loaded = false;
static u8 current_command_id = 0;

// Low-level port access

void write_port(Port port, u8 value) {
    hal::write8(APU_PORT0 + static_cast<u8>(port), value);
}

u8 read_port(Port port) {
    return hal::read8(APU_PORT0 + static_cast<u8>(port));
}

void write_port(u8 port, u8 value) {
    hal::write8(APU_PORT0 + (port & 0x03), value);
}

u8 read_port(u8 port) {
    return hal::read8(APU_PORT0 + (port & 0x03));
}

// Wait for a specific value on port 0
static void wait_port0(u8 expected) {
    while (read_port(Port::Port0) != expected) {
        // Spin
    }
}

// IPL ROM upload protocol
// The SPC700's IPL ROM provides a protocol for uploading code/data:
// 1. Wait for $AA on port 0 and $BB on port 1 (IPL ready)
// 2. Write destination address to ports 2-3
// 3. Write $CC to port 1, wait for $CC echo on port 0
// 4. Write data bytes to port 1, incrementing port 0 each time
// 5. To start execution, write $00 to port 1 (or continue with new address)

void init() {
    // Wait for IPL ROM to be ready
    // IPL signals ready with $AA on port 0 and $BB on port 1
    while (read_port(Port::Port0) != 0xAA || read_port(Port::Port1) != 0xBB) {
        // Spin
    }

    driver_loaded = false;
    current_command_id = 0;
}

bool is_ready() {
    if (!driver_loaded) {
        // Check for IPL ROM ready signal
        return read_port(Port::Port0) == 0xAA && read_port(Port::Port1) == 0xBB;
    }
    // Driver loaded - assume ready (or implement driver-specific check)
    return true;
}

void wait_ready() {
    while (!is_ready()) {
        // Spin
    }
}

void upload(u16 addr, const void* data, u16 size) {
    if (size == 0) return;

    const u8* bytes = static_cast<const u8*>(data);

    // Wait for IPL ready
    wait_ready();

    // Set destination address
    write_port(Port::Port2, static_cast<u8>(addr & 0xFF));
    write_port(Port::Port3, static_cast<u8>(addr >> 8));

    // Signal start of transfer (write $CC to port 1)
    write_port(Port::Port1, 0xCC);

    // Wait for acknowledgment
    wait_port0(0xCC);

    // Upload bytes
    u8 counter = 0;
    for (u16 i = 0; i < size; i++) {
        // Write data byte to port 1
        write_port(Port::Port1, bytes[i]);

        // Write counter to port 0
        write_port(Port::Port0, counter);

        // Wait for counter echo
        wait_port0(counter);

        counter++;
    }
}

void upload_and_run(u16 addr, const void* data, u16 size) {
    // Upload the data first
    upload(addr, data, size);

    // To execute: write start address and $00 to port 1
    // Actually, we write the address where we want to jump

    // Set execution address
    write_port(Port::Port2, static_cast<u8>(addr & 0xFF));
    write_port(Port::Port3, static_cast<u8>(addr >> 8));

    // Signal execution (non-zero followed by zero tells IPL to jump)
    u8 counter = read_port(Port::Port0) + 2;  // Skip past current counter
    write_port(Port::Port1, 0x00);  // Execute
    write_port(Port::Port0, counter);

    driver_loaded = true;
}

// Helper to send a command to the audio driver
static void send_command(u8 cmd, u8 param = 0) {
    if (!driver_loaded) return;

    // Increment command ID to signal new command
    current_command_id++;
    if (current_command_id == 0) current_command_id = 1;

    // Write parameter first, then command + ID
    write_port(Port::Port1, param);
    write_port(Port::Port0, cmd);
    write_port(Port::Port2, current_command_id);

    // Wait for acknowledgment (driver echoes command ID)
    while (read_port(Port::Port2) != current_command_id) {
        // Spin
    }
}

void play_music(u8 song_id) {
    send_command(command::PLAY, song_id);
}

void stop_music() {
    send_command(command::STOP);
}

void pause_music() {
    send_command(command::PAUSE);
}

void resume_music() {
    send_command(command::RESUME);
}

void set_music_volume(u8 volume) {
    send_command(command::SET_VOLUME, volume & 0x7F);
}

void play_sfx(u8 sfx_id, u8 priority) {
    // Encode priority in high bits if needed by driver
    send_command(command::PLAY_SFX, sfx_id);
    (void)priority;  // Would need driver support
}

void stop_sfx() {
    send_command(command::STOP_SFX);
}

void set_sfx_volume(u8 volume) {
    // Would need driver support - for now, use music volume
    send_command(command::SET_VOLUME, volume & 0x7F);
}

void set_tempo(u8 tempo) {
    send_command(command::SET_TEMPO, tempo);
}

bool is_music_playing() {
    if (!driver_loaded) return false;
    // Driver-specific: check status port
    return (read_port(Port::Port3) & 0x01) != 0;
}

u8 get_music_position() {
    if (!driver_loaded) return 0;
    // Driver-specific: read position from port
    return read_port(Port::Port3);
}

void upload_samples(u16 spc_addr, const void* brr_data, u16 size) {
    upload(spc_addr, brr_data, size);
}

void upload_sample_directory(u16 spc_addr, const void* dir_data, u16 size) {
    upload(spc_addr, dir_data, size);
}

void write_dsp(u8 reg, u8 value) {
    // This requires driver support - the driver must expose DSP write functionality
    // A simple protocol: write reg to port 2, value to port 3, command to port 0
    if (!driver_loaded) return;

    // This is a placeholder - actual implementation depends on driver protocol
    (void)reg;
    (void)value;
}

u8 read_dsp(u8 reg) {
    // This requires driver support
    if (!driver_loaded) return 0;

    // Placeholder
    (void)reg;
    return 0;
}

} // namespace snes::audio
