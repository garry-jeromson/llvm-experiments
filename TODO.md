# W65816 LLVM Backend TODO

## Recently Completed

### Backend Development
- [x] Coding standards audit - clang-format, error message conventions, static_cast
- [x] MC layer tests - 10 assembler tests for instruction encoding
- [x] Documentation - README.md, W65816Usage.rst, added to LLVM docs index
- [x] Pre-commit hook - Auto-format W65816 files on commit

### SNES SDK
- [x] Port stdlib functions (strlen, strcpy, strcmp, strncpy, strncmp, strcat, strchr)
- [x] String display driver (text.hpp, font_2bpp.s)
- [x] Sound driver API (audio.hpp, audio.cpp)
  - C++ API for SPC700 communication
  - Sound effect and music playback interface
  - Volume control
  - Demo application (audio_demo)
  - Note: Requires SPC700 binary for actual sound output
- [x] SuperFX driver API (superfx.hpp, superfx.cpp)
  - C++ API for GSU-1/GSU-2 coprocessor
  - Execution control, screen configuration
  - Memory access, high-speed mode support
  - Demo application (superfx_demo)
  - Note: Requires SuperFX programs for actual 3D rendering
- [x] SDK code quality review - Added named constants to replace magic numbers:
  - text.hpp/cpp: `SCREEN_COLS`, `SCREEN_ROWS`, `TAB_ALIGN_MASK`
  - ppu.hpp: `BRIGHTNESS_MASK`
  - audio.hpp: Added explanatory comments for counter wrapping logic
  - superfx.hpp: `HEIGHT_128/160/192`, `DEPTH_2BPP/4BPP/8BPP`

---

## LLVM Upstream Submission

Preparing the backend for inclusion in the official LLVM project.

- [ ] Draft RFC for LLVM Discourse
- [ ] Set up buildbot infrastructure
- [ ] Organize patches for incremental review (~9 patches)
- [ ] Add Maintainers.md entry

---

## SNES SDK

### High Priority
- [ ] Sound driver SPC700 binary - Need SPC700 assembler to compile the driver code
- [ ] SuperFX driver programs - Need actual SuperFX assembly programs to run

### Infrastructure
- [x] Linker scripts (`snes-sdk/linker_configs/`)
  - `lorom.cfg` - Standard LoROM (32KB single bank)
  - `hirom.cfg` - Standard HiROM (64KB)
  - `lorom-multibank.cfg` - Multi-bank LoROM (256KB, extendable)
- [x] ROM builder tooling (`snes-sdk/rom_builder/`)
  - `build_rom.py` supports lorom, hirom, multibank, superfx cart types
  - `fix_checksum.py` auto-detects LoROM/HiROM layout
  - `run_rom.py` launches ROMs in emulator
  - `requirements.txt` for Python dependencies
  - Makefile includes venv setup (`make venv`)
- [ ] Standard library stubs (minimal libc)

---

## Backend Polish (Lower Priority)

- [ ] Extend assembler addressing mode support (indexed modes for ADC/SBC/etc)
- [ ] Add disassembler tests
- [ ] Debug info (DWARF) support for emulator debugging

---

## Notes

### Sound Driver
The SNES uses a separate Sony SPC700 processor for audio.

**What's implemented:**
- `snes-sdk/include/snes/audio.hpp` - API header with SFX/music functions
- `snes-sdk/src/audio.cpp` - 65816-side driver implementation
- `snes-sdk/data/spc700_driver.s` - SPC700 driver source (needs SPC700 assembler)
- `snes-sdk/examples/audio_demo/` - Demo application

**To complete:**
- Compile SPC700 driver with an SPC700 assembler (not ca65, which is 65816-only)
- Options: bass assembler, WLA-DX, or a custom tool
- The SPC700 code provides tone generation and responds to commands from the 65816

### SuperFX Driver
The SuperFX (GSU-1/GSU-2) is a RISC coprocessor used in cartridges like Star Fox.

**What's implemented:**
- `snes-sdk/include/snes/superfx.hpp` - API header with execution control
- `snes-sdk/src/superfx.cpp` - 65816-side driver implementation
- `snes-sdk/examples/superfx_demo/` - Demo application

**API features:**
- Detection and version identification (GSU-1 vs GSU-2)
- Execution control (start, stop, wait)
- Screen configuration (resolution, color depth)
- Memory access (read/write RAM)
- High-speed mode (21.4 MHz for GSU-2)
- IRQ handling

**To complete:**
- Compile actual SuperFX programs (RISC assembly)
- Options: bass assembler with SuperFX support, or custom tool
- The SuperFX code runs on a separate RISC processor with its own instruction set
