#pragma once

#include <snes/hal.hpp>
#include <cstring>
#include <utility>

namespace snes::testing {

// Maximum number of write operations to record
constexpr int MAX_WRITES = 256;

// Fake register access for testing
// Records all writes and returns preset values for reads
// This is a "fake" (not a mock) because it implements actual behavior
// that can be swapped in during testing
struct FakeRegisterAccess : hal::IRegisterAccess {
    // Recorded writes
    struct WriteRecord {
        u32 addr;
        u8 value;
        bool is_16bit;
    };
    WriteRecord writes[MAX_WRITES];
    int write_count = 0;

    // Preset read values (sparse array simulation using linear search)
    // For testing, this is fine since we don't have many entries
    struct ReadEntry {
        u32 addr;
        u16 value;  // Holds u8 or u16
        bool valid;
    };
    static constexpr int MAX_READS = 64;
    ReadEntry read_values[MAX_READS];
    int read_count = 0;

    FakeRegisterAccess() {
        clear();
    }

    void clear() {
        write_count = 0;
        read_count = 0;
        for (int i = 0; i < MAX_READS; i++) {
            read_values[i].valid = false;
        }
    }

    // Set a value to be returned when reading an address
    void set_read_value(u32 addr, u8 value) {
        for (int i = 0; i < read_count; i++) {
            if (read_values[i].addr == addr) {
                read_values[i].value = value;
                return;
            }
        }
        if (read_count < MAX_READS) {
            read_values[read_count].addr = addr;
            read_values[read_count].value = value;
            read_values[read_count].valid = true;
            read_count++;
        }
    }

    void set_read_value16(u32 addr, u16 value) {
        set_read_value(addr, static_cast<u8>(value & 0xFF));
        set_read_value(addr + 1, static_cast<u8>(value >> 8));
    }

    // IRegisterAccess implementation
    void write8(u32 addr, u8 val) override {
        if (write_count < MAX_WRITES) {
            writes[write_count].addr = addr;
            writes[write_count].value = val;
            writes[write_count].is_16bit = false;
            write_count++;
        }
    }

    u8 read8(u32 addr) override {
        for (int i = 0; i < read_count; i++) {
            if (read_values[i].addr == addr && read_values[i].valid) {
                return static_cast<u8>(read_values[i].value);
            }
        }
        return 0;  // Default return value
    }

    void write16(u32 addr, u16 val) override {
        write8(addr, static_cast<u8>(val & 0xFF));
        write8(addr + 1, static_cast<u8>(val >> 8));
    }

    u16 read16(u32 addr) override {
        u8 lo = read8(addr);
        u8 hi = read8(addr + 1);
        return static_cast<u16>(lo | (hi << 8));
    }

    // Test helpers

    // Check if a specific value was written to an address
    bool wrote(u32 addr, u8 val) const {
        for (int i = 0; i < write_count; i++) {
            if (writes[i].addr == addr && writes[i].value == val) {
                return true;
            }
        }
        return false;
    }

    // Check if any write was made to an address
    bool wrote_to(u32 addr) const {
        for (int i = 0; i < write_count; i++) {
            if (writes[i].addr == addr) {
                return true;
            }
        }
        return false;
    }

    // Get the last value written to an address
    u8 last_write(u32 addr) const {
        for (int i = write_count - 1; i >= 0; i--) {
            if (writes[i].addr == addr) {
                return writes[i].value;
            }
        }
        return 0;
    }

    // Count writes to an address
    int count_writes(u32 addr) const {
        int count = 0;
        for (int i = 0; i < write_count; i++) {
            if (writes[i].addr == addr) {
                count++;
            }
        }
        return count;
    }

    // Get all writes in order
    int get_writes(u32 addr, u8* out_values, int max_count) const {
        int count = 0;
        for (int i = 0; i < write_count && count < max_count; i++) {
            if (writes[i].addr == addr) {
                out_values[count++] = writes[i].value;
            }
        }
        return count;
    }

    // Verify a sequence of writes occurred
    bool verify_write_sequence(const std::pair<u32, u8>* expected, int count) const {
        if (count > write_count) return false;

        int match_idx = 0;
        for (int i = 0; i < write_count && match_idx < count; i++) {
            if (writes[i].addr == expected[match_idx].first &&
                writes[i].value == expected[match_idx].second) {
                match_idx++;
            }
        }
        return match_idx == count;
    }
};

// Global fake for testing
inline FakeRegisterAccess* g_fake_hal = nullptr;

inline void setup_fake_hal() {
    static FakeRegisterAccess fake;
    g_fake_hal = &fake;
    hal::set_hal(fake);
}

inline FakeRegisterAccess& get_fake() {
    return *g_fake_hal;
}

} // namespace snes::testing
