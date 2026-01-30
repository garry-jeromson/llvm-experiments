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

## Build Commands

```bash
make build-fast    # Build essential tools (llc, opt, clang)
make rebuild       # Incremental rebuild after changes

# Compile test
./build/bin/llc -march=w65816 test_file.ll -o -

# Generate object file
./build/bin/llc -march=w65816 -filetype=obj test_file.ll -o test_file.o
```

## Testing

**IMPORTANT: All changes must pass tests before committing.**

```bash
# Run W65816 tests only (REQUIRED after every change)
ninja -C build check-llvm-codegen-w65816

# Alternative using make
make test-w65816

# Workflow after making changes:
make rebuild && ninja -C build check-llvm-codegen-w65816
```

### Test Files

| File | Coverage |
|------|----------|
| `arithmetic.ll` | add, sub, inc, dec |
| `logical.ll` | and, or, xor |
| `shifts.ll` | constant and variable shifts |
| `memory.ll` | loads, stores, indirect addressing |
| `memory-ops.ll` | memory inc/dec/shift optimization |
| `dp-indirect.ll` | direct page indirect addressing |
| `control-flow.ll` | calls, returns, branches |
| `select.ll` | conditional select (signed/unsigned comparisons) |
| `basic.ll` | basic operations |
| `mul-div-rem.ll` | multiply, divide, remainder (libcalls) |
| `interrupt.ll` | interrupt handler support (RTI, register save/restore) |

---

## W65816 Architecture Quick Reference

- **Registers**: A (accumulator), X, Y (index), SP (stack), D (direct page)
- **Data size**: 8-bit or 16-bit (mode switchable via M/X flags)
- **Pointers**: 16-bit (bank 0) or 24-bit (with bank register)
- **Key addressing modes**:
  - Absolute: `lda addr`
  - Stack-relative: `lda n,s`
  - Indexed: `lda addr,x` / `lda addr,y`
  - Direct page: `lda dp`
  - Indirect: `lda (dp)` / `lda (dp),y` / `lda (n,s),y`
  - Long: `lda $123456` (24-bit)

---

## Implementation Status

### What Works

**Core Operations:**
- Register file with 8/16-bit subregs
- Arithmetic: ADC, SBC, INC, DEC (+ library calls for MUL, DIV, REM)
- Logical: AND, ORA, EOR
- Shifts: ASL, LSR, ROL, ROR (constant and variable amounts)
- All comparison conditions (signed and unsigned, including compound UGT/ULE)

**Memory & Addressing:**
- Load/store: LDA, STA, LDX, LDY, STX, STY
- Absolute, stack-relative, indexed (X and Y), direct page
- Long (24-bit) addressing for `.fardata`, `.rodata`, `.romdata` sections
- Pointer dereference via stack-relative indirect `(n,s),y`
- Direct page indirect `($dp)` and `($dp),y` for efficient pointer access in zero page
- 8-bit loads/stores with automatic SEP/REP mode switching

**Control Flow:**
- Branches: BNE, BEQ, BCS, BCC, BMI, BPL, BVS, BVC, BRA
- Calls: JSR, RTS, RTI
- Interrupt handlers with `__attribute__((interrupt))`

**Stack & ABI:**
- Stack frame management (prologue/epilogue)
- Register spilling via LDA_sr/STA_sr
- First 3 args in A, X, Y; additional args on stack

**Code Generation:**
- MC layer with ELF object generation
- Runtime library for MUL/DIV/REM (`runtime/w65816_runtime.s`)
- Assembly parser with all indirect addressing modes:
  - `(dp)`, `(dp),y`, `(dp,x)` - DP indirect
  - `[dp]`, `[dp],y` - DP indirect long
  - `(n,s),y` - Stack relative indirect
  - `(addr)`, `(addr,x)`, `[addr]` - JMP indirect

### Known Limitations

**Register Pressure:**
- Only A, X, Y available for general use
- Complex phi nodes with high register pressure may fail
- Indexed stores with conflicting argument order may generate incorrect code
  - Workaround: Order function arguments so value is first (in A) and index is second (in X)

**Suboptimal Code Generation:**
- ADD16rr/SUB16rr uses stack-relative addressing (push, operate, pull overhead)
- Self-comparison generates unnecessary code
- Memory ROL/ROR not yet optimized (INC/DEC/ASL/LSR are optimized)

**Not Implemented:**
- Vararg support
- 32-bit return values (A:X pair) untested
- Runtime 8/16-bit mode switching (compile-time flags only)
- `$` hex prefix in assembly (use `0x` instead)

---

## Remaining Work

### Low Priority
1. **Vararg Support** - For printf-style functions
2. **32-bit Returns** - Test and fix A:X pair returns
3. **`$` Hex Prefix** - Add support for `$FF` syntax (currently requires `0xFF`)
4. **ROL/ROR Memory** - Memory rotate patterns (INC/DEC/ASL/LSR done)

---

## Feature Reference

### 8/16-bit Mode (Compile-Time)

```bash
llc -march=w65816 test.ll                    # 16-bit mode (default)
llc -march=w65816 -mattr=+acc8bit test.ll    # 8-bit accumulator
llc -march=w65816 -mattr=+idx8bit test.ll    # 8-bit index registers
```

### Long Addressing

Globals in these sections use 24-bit addressing:
- `.fardata`, `.rodata`, `.romdata`, `.bank*`

### Interrupt Handlers

```c
__attribute__((interrupt))
void irq_handler(void) {
    // Prologue: rep #48, pha, phx, phy
    // Epilogue: rep #48, ply, plx, pla, rti
}
```

### Runtime Library

```bash
# Assemble runtime library
ca65 --cpu 65816 -o w65816_runtime.o src/llvm-project/llvm/lib/Target/W65816/runtime/w65816_runtime.s

# Link with your code
```

Provides: `__mulhi3`, `__divhi3`, `__udivhi3`, `__modhi3`, `__umodhi3`

---

## Code Generation Examples

### Array Access
```asm
; array_load(i16 %idx):
asl a               ; idx * 2
tax                 ; put index in X
lda global_array,x  ; indexed load
rts
```

### Direct Page (Zero Page)
```llvm
@zp_var = global i16 42, section ".zeropage"
; Generates 2-byte instructions (opcode 0xA5/0x85)
```

### Direct Page Indirect (Pointer in Zero Page)
```llvm
@dp_ptr = global ptr null, section ".zeropage"
; Dereferencing uses efficient ($dp) addressing:
;   lda (dp_ptr)     ; load through DP pointer
;   sta (dp_ptr)     ; store through DP pointer
;   lda (dp_ptr),y   ; indexed load: ptr[i]
```

### Stack Spills
```asm
sta 3,s    ; 2-byte Folded Spill
lda 3,s    ; 2-byte Folded Reload
```
