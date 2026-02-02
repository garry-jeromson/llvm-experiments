#pragma once

// SNES Audio API - Sound driver interface for the SPC700 audio processor
//
// The SNES uses a Sony SPC700 processor for audio, which is completely separate
// from the main 65816 CPU. Communication happens via 4 I/O ports (APUIO0-3).
//
// This is a header-only implementation to avoid register pressure issues
// with the W65816's limited register set (A, X, Y only).

#include "types.hpp"
#include "hal.hpp"
#include "registers.hpp"

namespace snes {
namespace audio {

// Sound effect IDs (defined by the SPC700 driver)
enum SoundEffect : u8 {
    SFX_NONE     = 0,
    SFX_BEEP     = 1,   // Simple beep tone
    SFX_CLICK    = 2,   // UI click sound
    SFX_CONFIRM  = 3,   // Confirmation sound
    SFX_CANCEL   = 4,   // Cancel/back sound
    SFX_JUMP     = 5,   // Jump sound effect
    SFX_COIN     = 6,   // Collect item sound
    SFX_HURT     = 7,   // Damage sound
};

// Music track IDs
enum MusicTrack : u8 {
    MUSIC_NONE   = 0,   // No music (stop)
    MUSIC_TITLE  = 1,   // Title screen music
    MUSIC_GAME   = 2,   // Gameplay music
    MUSIC_MENU   = 3,   // Menu music
};

// APU communication commands (sent to APUIO0)
namespace cmd {
    constexpr u8 NOP         = 0x00;  // No operation
    constexpr u8 PLAY_SFX    = 0x01;  // Play sound effect (APUIO1 = SFX ID)
    constexpr u8 PLAY_MUSIC  = 0x02;  // Play music track (APUIO1 = track ID)
    constexpr u8 STOP_MUSIC  = 0x03;  // Stop music playback
    constexpr u8 SET_VOLUME  = 0x04;  // Set master volume (APUIO1 = 0-127)
    constexpr u8 SET_SFX_VOL = 0x05;  // Set SFX volume (APUIO1 = 0-127)
    constexpr u8 SET_MUS_VOL = 0x06;  // Set music volume (APUIO1 = 0-127)
    constexpr u8 STOP_ALL    = 0x07;  // Stop all audio
    constexpr u8 READY       = 0xAA;  // Driver ready acknowledgement
}

// Global state variables (defined in crt0.s)
extern volatile u8 g_audio_initialized;
extern volatile u8 g_audio_master_volume;
extern volatile u8 g_audio_command_counter;

// ============================================================================
// Direct APU I/O port access
// ============================================================================

inline void write_apuio0(u8 val) { hal::write8(reg::APUIO0::address, val); }
inline void write_apuio1(u8 val) { hal::write8(reg::APUIO1::address, val); }
inline void write_apuio2(u8 val) { hal::write8(reg::APUIO2::address, val); }
inline void write_apuio3(u8 val) { hal::write8(reg::APUIO3::address, val); }
inline u8 read_apuio0() { return hal::read8(reg::APUIO0::address); }
inline u8 read_apuio1() { return hal::read8(reg::APUIO1::address); }
inline u8 read_apuio2() { return hal::read8(reg::APUIO2::address); }
inline u8 read_apuio3() { return hal::read8(reg::APUIO3::address); }

// ============================================================================
// Simple command interface (no loops to avoid register pressure)
// ============================================================================

// Send a raw command to the APU (no handshaking - fire and forget)
inline void send_raw_command(u8 cmd_nibble, u8 param) {
    // Increment command counter (used by SPC700 driver to detect new commands)
    // Counter wraps 1→255→1, avoiding 0 which indicates "no command"
    g_audio_command_counter = g_audio_command_counter + 1;
    if (g_audio_command_counter == 0) g_audio_command_counter = 1;

    write_apuio1(param);
    u8 cmd_byte = (cmd_nibble << 4) | (g_audio_command_counter & 0x0F);
    write_apuio0(cmd_byte);
}

// ============================================================================
// Initialization
// ============================================================================

// Initialize the audio system (simple version - just marks as initialized)
// Full initialization requires SPC700 driver to be loaded
inline bool init() {
    // For now, just mark as initialized
    // Real initialization would wait for SPC700 driver handshake
    g_audio_initialized = 1;
    g_audio_master_volume = 127;
    g_audio_command_counter = 0;
    return true;
}

// Check if audio system is initialized
inline bool is_ready() {
    return g_audio_initialized != 0;
}

// ============================================================================
// Sound Effects
// ============================================================================

// Play a sound effect
inline void play_sfx(u8 sfx) {
    if (g_audio_initialized == 0) return;
    send_raw_command(cmd::PLAY_SFX, sfx);
}

// Play a sound effect (type-safe version)
inline void play_sfx(SoundEffect sfx) {
    play_sfx(static_cast<u8>(sfx));
}

// ============================================================================
// Music
// ============================================================================

// Play a music track
inline void play_music(u8 track) {
    if (g_audio_initialized == 0) return;
    send_raw_command(cmd::PLAY_MUSIC, track);
}

// Play a music track (type-safe version)
inline void play_music(MusicTrack track) {
    play_music(static_cast<u8>(track));
}

// Stop the currently playing music
inline void stop_music() {
    if (g_audio_initialized == 0) return;
    send_raw_command(cmd::STOP_MUSIC, 0);
}

// ============================================================================
// Volume Control
// ============================================================================

// Set master volume (0-127)
inline void set_master_volume(u8 volume) {
    if (g_audio_initialized == 0) return;
    u8 vol = volume & 0x7F;
    g_audio_master_volume = vol;
    send_raw_command(cmd::SET_VOLUME, vol);
}

// Get current master volume
inline u8 get_master_volume() {
    return g_audio_master_volume;
}

// Set sound effects volume (0-127)
inline void set_sfx_volume(u8 volume) {
    if (g_audio_initialized == 0) return;
    send_raw_command(cmd::SET_SFX_VOL, volume & 0x7F);
}

// Set music volume (0-127)
inline void set_music_volume(u8 volume) {
    if (g_audio_initialized == 0) return;
    send_raw_command(cmd::SET_MUS_VOL, volume & 0x7F);
}

// ============================================================================
// Playback Control
// ============================================================================

// Stop all audio (music and sound effects)
inline void stop_all() {
    if (g_audio_initialized == 0) return;
    send_raw_command(cmd::STOP_ALL, 0);
}

} // namespace audio
} // namespace snes
