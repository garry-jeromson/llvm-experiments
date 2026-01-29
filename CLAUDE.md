# W65816 LLVM Backend Project

This project is developing an LLVM backend for the **W65816** 16-bit microprocessor (used in the Super Nintendo/SNES).

## Project Structure

```
llvm-experiments/
├── Makefile                    # Build workflow (make deps, clone, configure, build-fast)
├── build/                      # Build output (llc, clang, opt binaries)
├── src/llvm-project/           # LLVM monorepo
│   └── llvm/lib/Target/W65816/ # Our backend implementation
│   └── llvm/test/CodeGen/W65816/ # Backend tests (FileCheck-based)
└── CLAUDE.md                   # This file
```

## Key Backend Files

Location: `src/llvm-project/llvm/lib/Target/W65816/`

| File | Purpose |
|------|---------|
| `W65816.td` | Top-level target definition |
| `W65816RegisterInfo.td` | Register file (A, X, Y, SP, D, DBR, PBR) |
| `W65816InstrInfo.td` | Instruction definitions and patterns |
| `W65816CallingConv.td` | Calling convention (args in A, X, Y) |
| `W65816ISelLowering.cpp` | DAG lowering |
| `W65816ISelDAGToDAG.cpp` | DAG instruction selection |
| `W65816ExpandPseudo.cpp` | Pseudo-instruction expansion |
| `W65816FrameLowering.cpp` | Stack frame management |
| `KNOWN_LIMITATIONS.md` | Documented limitations |

## Build Commands

```bash
make build-fast    # Build essential tools (llc, opt, clang)
make rebuild       # Incremental rebuild after changes

# Compile test
./build/bin/llc -march=w65816 test_file.ll -o -
```

## W65816 Architecture Quick Reference

- **Registers**: A (accumulator), X, Y (index), SP (stack), D (direct page)
- **Data size**: 8-bit or 16-bit (mode switchable)
- **Pointers**: 16-bit (bank 0) or 24-bit (with bank register)
- **Key addressing modes**:
  - Absolute: `lda addr`
  - Stack-relative: `lda n,s`
  - Indexed: `lda addr,x` / `lda addr,y`
  - Direct page: `lda dp`
  - Indirect: `lda (dp)` / `lda (dp),y` / `lda (n,s),y`

## Implementation Status

### Completed (~75%)
- Register file with 8/16-bit subregs
- Load/store: LDA, STA, LDX, LDY, STX, STY (multiple addressing modes)
- Arithmetic: ADC, SBC, INC, DEC
- Logical: AND, ORA, EOR
- Shifts: ASL, LSR, ROL, ROR
- Branches: BNE, BEQ, BCS, BCC, BMI, BPL, BVS, BRA
- Calls: JSR, RTS
- Stack operations: PHA, PLA, PHX, PLX, PHY, PLY
- Signed/unsigned comparisons
- Stack frame management (prologue/epilogue)
- Indirect addressing (via stack-relative indirect)
- MC layer with ELF object generation

### Known Issues
- **8-bit loads with address math**: Causes issues with pointer loads (double deref fails)
- **Limited registers**: Only A, X, Y available; complex operations may run out
- **No 8/16-bit mode switching** implemented yet

### Not Implemented
- 8-bit/16-bit mode switching (REP/SEP partially there)
- Long (24-bit) addressing
- Multiply/divide (need library calls)
- Interrupt handling

## Current Work (Jan 2025)

### Indirect Addressing - FIXED ✓

All indirect addressing modes now work:
- `*ptr` load/store ✓
- `ptr[i]` with constant index ✓
- `ptr[i]` with variable index ✓
- `**ptr` double dereference ✓

**Fix applied:** Changed data layout from `p:16:8` to `p:16:16` (16-bit pointer alignment) to prevent LLVM from decomposing 16-bit pointer loads into 8-bit loads.

## Testing

**IMPORTANT: All changes must pass tests before committing.**

Tests are located in `src/llvm-project/llvm/test/CodeGen/W65816/` and use LLVM's standard FileCheck infrastructure.

### Running Tests

```bash
# Run W65816 tests only (REQUIRED after every change)
ninja -C build check-llvm-codegen-w65816

# Alternative using make
make test-w65816

# Run all CodeGen tests (includes W65816)
ninja -C build check-llvm-codegen

# Run full LLVM test suite
ninja -C build check-llvm
```

### Test Files

| File | Coverage |
|------|----------|
| `arithmetic.ll` | add, sub, inc, dec |
| `logical.ll` | and, or, xor |
| `shifts.ll` | constant and variable shifts |
| `memory.ll` | loads, stores, indirect addressing |
| `control-flow.ll` | calls, returns, branches |
| `basic.ll` | basic operations |

### Writing Tests

Tests use FileCheck directives:
```llvm
; RUN: llc -march=w65816 < %s | FileCheck %s

; CHECK-LABEL: function_name:
; CHECK: expected_instruction
define i16 @function_name(i16 %a) {
  ; ... IR ...
}
```

### Workflow

```bash
# After making changes:
make rebuild && ninja -C build check-llvm-codegen-w65816
```

## Manual Testing

```bash
# Compile and view assembly
./build/bin/llc -march=w65816 test_file.ll -o -

# Generate object file
./build/bin/llc -march=w65816 -filetype=obj test_file.ll -o test_file.o
```

## Next Steps

1. **Fix setcc legalization** - Conditional branches crash (icmp/br patterns)
2. **8-bit mode switching** - REP/SEP instructions for mode changes
3. **Multiply/divide support** - Library call expansion
4. **Assembly parser** - Enable inline asm testing
