#pragma once

#include "types.hpp"

namespace snes::audio {

// APU communication ports
// The SNES communicates with the SPC700 via 4 bidirectional ports
// CPU writes to $2140-$2143, SPC700 reads from $F4-$F7
// SPC700 writes to $F4-$F7, CPU reads from $2140-$2143

// Port numbers
enum class Port : u8 {
    Port0 = 0,
    Port1 = 1,
    Port2 = 2,
    Port3 = 3
};

// Initialize audio system
// This uploads the audio driver to SPC700 RAM
void init();

// Check if audio system is ready
bool is_ready();

// Wait for audio system to be ready
void wait_ready();

// Low-level port access
void write_port(Port port, u8 value);
u8 read_port(Port port);

// Convenience overloads with u8 port number
void write_port(u8 port, u8 value);
u8 read_port(u8 port);

// Upload data to SPC700 RAM
// Uses IPL ROM upload protocol
// addr: Destination address in SPC700 RAM (0x0000-0xFFFF)
// data: Pointer to data in main RAM
// size: Number of bytes to upload
void upload(u16 addr, const void* data, u16 size);

// Upload and execute code
// After upload, jumps to the specified address
void upload_and_run(u16 addr, const void* data, u16 size);

// Music control (requires compatible audio driver)
// These use a simple protocol: Port0 = command, Port1 = parameter

namespace command {
    constexpr u8 NOP        = 0x00;
    constexpr u8 PLAY       = 0x01;  // Play song (param = song ID)
    constexpr u8 STOP       = 0x02;  // Stop music
    constexpr u8 PAUSE      = 0x03;  // Pause music
    constexpr u8 RESUME     = 0x04;  // Resume paused music
    constexpr u8 SET_VOLUME = 0x05;  // Set master volume (param = 0-127)
    constexpr u8 PLAY_SFX   = 0x10;  // Play sound effect (param = SFX ID)
    constexpr u8 STOP_SFX   = 0x11;  // Stop sound effect
    constexpr u8 SET_TEMPO  = 0x20;  // Set tempo multiplier (param = 0-255)
}

// Play a song
void play_music(u8 song_id);

// Stop music playback
void stop_music();

// Pause music (can be resumed)
void pause_music();

// Resume paused music
void resume_music();

// Set master music volume (0-127)
void set_music_volume(u8 volume);

// Play a sound effect
// priority: Higher priority SFX can interrupt lower priority
void play_sfx(u8 sfx_id, u8 priority = 0);

// Stop all sound effects
void stop_sfx();

// Set sound effect volume (0-127)
void set_sfx_volume(u8 volume);

// Set tempo multiplier (128 = normal speed)
void set_tempo(u8 tempo);

// Check if music is currently playing
bool is_music_playing();

// Get current song position (driver-dependent)
u8 get_music_position();

// Upload BRR sample data to SPC700 RAM
// Sample data should be in BRR format (9 bytes per block)
void upload_samples(u16 spc_addr, const void* brr_data, u16 size);

// Upload instrument/sample directory
// The directory tells the DSP where samples are located
void upload_sample_directory(u16 spc_addr, const void* dir_data, u16 size);

// Direct DSP register write (requires driver support)
// reg: DSP register address (0x00-0x7F)
// value: Value to write
void write_dsp(u8 reg, u8 value);

// Direct DSP register read (requires driver support)
u8 read_dsp(u8 reg);

// DSP register addresses
namespace dsp {
    // Voice registers (X = voice 0-7, multiply by 0x10 for voice N)
    constexpr u8 VOLL   = 0x00;  // Left volume
    constexpr u8 VOLR   = 0x01;  // Right volume
    constexpr u8 PITCHL = 0x02;  // Pitch (low byte)
    constexpr u8 PITCHH = 0x03;  // Pitch (high byte)
    constexpr u8 SRCN   = 0x04;  // Sample source number
    constexpr u8 ADSR1  = 0x05;  // ADSR settings 1
    constexpr u8 ADSR2  = 0x06;  // ADSR settings 2
    constexpr u8 GAIN   = 0x07;  // Gain settings
    constexpr u8 ENVX   = 0x08;  // Current envelope value (read-only)
    constexpr u8 OUTX   = 0x09;  // Current waveform value (read-only)

    // Global registers
    constexpr u8 MVOLL  = 0x0C;  // Master volume left
    constexpr u8 MVOLR  = 0x1C;  // Master volume right
    constexpr u8 EVOLL  = 0x2C;  // Echo volume left
    constexpr u8 EVOLR  = 0x3C;  // Echo volume right
    constexpr u8 KON    = 0x4C;  // Key on (play voices)
    constexpr u8 KOFF   = 0x5C;  // Key off (release voices)
    constexpr u8 FLG    = 0x6C;  // Flags (noise clock, echo, mute, reset)
    constexpr u8 ENDX   = 0x7C;  // Voice end flags (read-only)

    constexpr u8 EFB    = 0x0D;  // Echo feedback
    constexpr u8 PMON   = 0x2D;  // Pitch modulation enable
    constexpr u8 NON    = 0x3D;  // Noise enable
    constexpr u8 EON    = 0x4D;  // Echo enable
    constexpr u8 DIR    = 0x5D;  // Sample directory offset (page in SPC RAM)
    constexpr u8 ESA    = 0x6D;  // Echo buffer start address (page)
    constexpr u8 EDL    = 0x7D;  // Echo delay (4ms units, 0-15)

    // FIR filter coefficients (echo)
    constexpr u8 FIR0   = 0x0F;
    constexpr u8 FIR1   = 0x1F;
    constexpr u8 FIR2   = 0x2F;
    constexpr u8 FIR3   = 0x3F;
    constexpr u8 FIR4   = 0x4F;
    constexpr u8 FIR5   = 0x5F;
    constexpr u8 FIR6   = 0x6F;
    constexpr u8 FIR7   = 0x7F;
}

} // namespace snes::audio
