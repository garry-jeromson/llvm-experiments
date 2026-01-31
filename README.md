# W65816 LLVM Backend

An LLVM backend for the WDC 65816 16-bit microprocessor, as used in the Super Nintendo (SNES).

## Overview

This project adds W65816 target support to LLVM, enabling:
- Compilation of C code to W65816 assembly via Clang
- LLVM IR to W65816 code generation via LLC
- ELF object file output with relocations

## Project Structure

```
llvm-experiments/
├── src/llvm-project/              # LLVM monorepo (submodule)
│   └── llvm/lib/Target/W65816/    # Backend implementation
│   └── llvm/test/CodeGen/W65816/  # FileCheck tests
├── tools/816ce/                   # 65816 CPU emulator (submodule)
├── test/integration/              # Execution-based integration tests
├── docs/                          # Additional documentation
└── snes/                          # SNES demo programs
```

## Getting Started

### Clone

```bash
git clone --recurse-submodules https://github.com/garry-jeromson/llvm-experiments.git
cd llvm-experiments
```

### Build

```bash
# Install dependencies (macOS)
make deps

# Configure and build (first time)
make configure
make build-fast

# Incremental rebuild after changes
make rebuild
```

This builds `llc`, `clang`, and `opt` in the `build/bin/` directory.

### Test

```bash
# Run W65816 FileCheck tests
make test-w65816

# Run integration tests (compiles and executes in CPU emulator)
make test-integration
```

## Usage

### Compile C to W65816 Assembly

```bash
./build/bin/clang -target w65816-unknown-none -O2 -S input.c -o output.s
```

### Compile LLVM IR to Assembly

```bash
./build/bin/llc -march=w65816 input.ll -o output.s
```

### Generate Object File

```bash
./build/bin/llc -march=w65816 -filetype=obj input.ll -o output.o
```

## Development Workflow

1. Make changes to backend files in `src/llvm-project/llvm/lib/Target/W65816/`
2. Rebuild: `make rebuild`
3. Run tests: `make test-w65816`
4. For execution verification: `make test-integration`

### Key Backend Files

| File | Purpose |
|------|---------|
| `W65816.td` | Top-level target definition |
| `W65816RegisterInfo.td` | Register definitions (A, X, Y, SP, D) |
| `W65816InstrInfo.td` | Instruction patterns |
| `W65816ISelLowering.cpp` | DAG lowering |
| `W65816ISelDAGToDAG.cpp` | Instruction selection |

## Architecture Notes

- **Registers**: A (accumulator), X, Y (index), SP (stack pointer), D (direct page)
- **Data width**: 8-bit or 16-bit (mode switchable)
- **Calling convention**: First 3 args in A, X, Y; rest on stack
- **Addressing modes**: Absolute, indexed, stack-relative, direct page, indirect

See `CLAUDE.md` for comprehensive implementation details.

## License

The LLVM modifications follow the LLVM license (Apache 2.0 with LLVM exceptions).
