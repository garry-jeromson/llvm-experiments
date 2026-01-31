# W65816 LLVM Backend Project

This project is developing an LLVM backend for the **W65816** 16-bit microprocessor (used in the Super Nintendo/SNES).

## Project Structure

```
llvm-experiments/
├── Makefile                       # Build workflow (make deps, clone, configure, build-fast)
├── build/                         # Build output (llc, clang, opt binaries)
├── src/llvm-project/              # LLVM monorepo (submodule)
│   └── llvm/lib/Target/W65816/    # Backend implementation
│   └── llvm/test/CodeGen/W65816/  # FileCheck tests
├── tools/816ce/                   # 65816 CPU emulator (submodule)
├── test/integration/              # Execution-based integration tests
├── docs/                          # Additional documentation
├── snes/                          # SNES demo programs
├── README.md                      # Quick-start guide
└── CLAUDE.md                      # This file (comprehensive reference)
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
| `W65816PeepholeOpt.cpp` | Peephole optimizations (eliminates redundant transfers) |
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
# Run W65816 FileCheck tests (uses llvm-lit)
make test-w65816

# Workflow after making changes:
make rebuild && make test-w65816
```

### Integration Tests (Execution Verification)

Integration tests compile LLVM IR to machine code and execute it in a CPU emulator (816CE)
to verify the generated code produces correct results.

```bash
# Run integration tests
make test-integration
```

Test files are in `test/integration/tests/*.ll` with format:
```llvm
; INTEGRATION-TEST
; EXPECT: 42
; SKIP: reason (optional, to skip a test)

target triple = "w65816-unknown-none"

define i16 @test_main() {
  ret i16 42
}
```

**Current Limitations:**
- Tests with internal function calls require relocation (supported)
- Tests with global variables require relocation (supported)
- Complex phi nodes with high register pressure may generate suboptimal code

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
| `varargs.ll` | variadic function support |
| `inline-asm-hex.ll` | Motorola-style integers ($hex, %binary) |
| `inline-asm-rotate.ll` | ROL/ROR rotate-through-carry |
| `far-calls.ll` | JSL/RTL far function calls |
| `dpframe.ll` | Direct Page frame allocation |
| `dpframe-overflow.ll` | DP frame 256-byte limit error |
| `edge-cases.ll` | Zero/max values, boundary conditions |
| `complex-control-flow.ll` | Nested calls, loops, phi nodes |
| `register-pressure.ll` | Spilling with A, X, Y pressure |
| `mode-8bit-ops.ll` | 8-bit load/store, truncation, extension |
| `stack-stress.ll` | Stack frames, value preservation across calls |
| `addressing-modes.ll` | Absolute, indexed, DP, immediate addressing |
| `special-instructions.ll` | BIT, XBA, TXY/TYX, PEA, SEP/REP |
| `global-data.ll` | Initialized arrays, struct access, const data |
| `switch.ll` | Switch statements (branch chains) |

### Integration Test Files

Location: `test/integration/tests/`

| Category | Tests |
|----------|-------|
| Arithmetic | `add`, `sub`, `inc`, `dec`, `neg`, `neg-variable`, `mul`, `double-neg` |
| Logical | `and`, `or`, `xor` |
| Shifts | `shl`, `lshr`, `ashr` |
| Comparisons | `icmp-eq`, `icmp-ne`, `icmp-slt`, `icmp-sgt`, `icmp-ult`, `icmp-ugt` |
| Unsigned cmp | `cmp-unsigned`, `cmp-unsigned-simple`, `cmp-args`, `cmp-edge-cases`, `int16-max-cmp` |
| Select | `select` |
| Type conversion | `sext`, `zext`, `trunc` |
| Memory | `store-load`, `global` |
| Control flow | `branch`, `loop` |
| Function calls | `call`, `nested-calls`, `many-args`, `call-shuffle-args` |
| Boundary values | `zero`, `large-immediate` |

### Integration Test Architecture

**Test Format**

Tests in `test/integration/tests/*.ll`:
- `; INTEGRATION-TEST` - marks file as an integration test
- `; EXPECT: N` - expected return value (i16)
- `; SKIP: reason` - skip test with reason (optional)
- `define i16 @test_main()` - required entry point

**Compilation Pipeline**

1. `llc -march=w65816 -filetype=obj` → ELF object file
2. Parse ELF sections (`.text`, `.data`) and symbol table
3. Apply relocations (`R_W65816_16`, `R_W65816_24`)
4. Build ROM image with startup code at $8000
5. Execute in 816CE CPU emulator
6. Compare result at $0000 with EXPECT value

**Memory Layout**

| Address | Content |
|---------|---------|
| $0000-$00FF | Zero page (result stored at $0000) |
| $0100-$01FF | Stack |
| $8000 | Startup code: `CLC, XCE, REP #$30, LDX #$01FF, TXS, JSR test_main, STA $00, STP` |
| $800E+ | Test code and data |
| $FFFC | Reset vector → $8000 |

**Relocation Support**

The test runner applies relocations to link symbols:
- `R_W65816_16`: 16-bit absolute address
- `R_W65816_24`: 24-bit long address (bank + offset)

This allows tests with function calls and global variables to work correctly.

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
- DP indexed modes: `LDX dp,Y` and `LDY dp,X` for cross-index register access
- Long (24-bit) addressing for `.fardata`, `.rodata`, `.romdata` sections
- Pointer dereference via stack-relative indirect `(n,s),y`
- Direct page indirect `($dp)` and `($dp),y` for efficient pointer access in zero page
- 8-bit loads/stores with automatic SEP/REP mode switching

**Control Flow:**
- Branches: BNE, BEQ, BCS, BCC, BMI, BPL, BVS, BVC, BRA
- Calls: JSR/RTS (normal), JSL/RTL (far), RTI (interrupt)
- Interrupt handlers with `__attribute__((interrupt))`
- Far functions with `__attribute__((w65816_farfunc))`

**Stack & ABI:**
- Stack frame management (prologue/epilogue)
- Register spilling via LDA_sr/STA_sr
- First 3 args in A, X, Y; additional args on stack
- Varargs support (va_start, va_arg, va_end, va_copy)

**Code Generation:**
- Native Clang target: `clang -target w65816-unknown-none`
- MC layer with ELF object generation
- Runtime library for MUL/DIV/REM (`runtime/w65816_runtime.s`)
- Motorola-style integers in inline assembly: `$FF` (hex), `%11110000` (binary)
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

**Phi Node Issues:**
- Complex phi nodes with high register pressure may generate suboptimal code
- Workaround: Use `select` instruction instead of phi nodes for conditional values when possible

**Suboptimal Code Generation:**
- Conditional branches may have suboptimal block layout (mitigated by peephole pass)

**Semantic Mismatches:**
- Memory ROL/ROR cannot be optimized because W65816's ROL/ROR are "rotate through carry"
  (17-bit rotation including carry flag), while LLVM's ROTL/ROTR are true N-bit rotations.
  For rotate-through-carry operations, use inline assembly.

**Not Implemented:**
- 32-bit integer operations (i32 requires manual 16-bit pair handling)
- Runtime 8/16-bit mode switching (compile-time flags only)

---

## Manual Audit Results

Verified against WDC W65816 Programming Manual. **Coverage: ~95%**

### Chapter Verification Status

| Chapters | Topic | Status |
|----------|-------|--------|
| 1-4 | Architecture (registers, flags, modes) | ✓ Consistent |
| 5-6 | Data Movement (load/store, push/pull, transfer) | ✓ Consistent |
| 7 | Simple Addressing (immediate, absolute, DP, indexed) | ✓ Consistent |
| 8-10 | Arithmetic, Logic, Control Flow | ✓ Consistent |
| 11 | Complex Addressing (indirect, stack-relative, long) | ✓ Consistent |
| 12-13 | Subroutines & Interrupts (JSR/JSL, RTS/RTL, RTI) | ✓ Consistent |
| 17-19 | Complete Instruction Set | ✓ 90+ of 94 instructions |

### Direct Page Coverage

- **43+ DP-specific instructions** defined (load/store, indexed, indirect, arithmetic, logical, shifts)
- **Section detection** for `.zeropage`, `.directpage`, `.zp` (auto-selects 2-byte instructions)
- **DP frame allocation** via `__attribute__((annotate("w65816_dpframe")))`
- **D register handling** with PHD/PLD save/restore, `-mattr=+assume-d0` optimization

### All 24 Addressing Modes Implemented

Immediate, Absolute, DP, DP Indexed X/Y, Absolute Indexed X/Y, DP Indirect, DP Indirect Y,
DP Indexed Indirect X, DP Indirect Long, DP Indirect Long Y, Stack Relative,
Stack Relative Indirect Y, Absolute Indirect, Absolute Indexed Indirect, Long, Long Indexed X.

### Acceptable Limitations (By Design)

| Feature | Notes |
|---------|-------|
| Emulation mode | Native mode only (SNES uses native) |
| DBR/PBR management | Manual control via inline assembly |
| Branch relaxation | Assembler handles out-of-range branches |
| Decimal mode | CLD/SED defined, no BCD codegen (not needed for C) |

---

## Remaining Work

### Optimization & Polish
1. ~~**ADD16rr/SUB16rr optimization**~~ - ✅ Done: Uses DP scratch at $FE (saves 4 cycles per operation)
1b. ~~**AND16rr/OR16rr/XOR16rr optimization**~~ - ✅ Done: Uses DP scratch at $FE (saves 4 cycles per operation)
2. ~~**Test coverage expansion**~~ - ✅ Done:
   - ✅ edge-cases.ll - Zero/max values, boundary conditions
   - ✅ complex-control-flow.ll - Nested calls, loops, phi nodes
   - ✅ register-pressure.ll - Operations requiring more than A, X, Y; spill/reload
   - ✅ mode-8bit-ops.ll - 8-bit load/store, truncation, extension, mode switching
   - ✅ stack-stress.ll - Stack frames, value preservation across calls
   - ✅ addressing-modes.ll - Absolute, indexed, DP, immediate addressing
   - ✅ special-instructions.ll - BIT, XBA, TXY/TYX, PEA, SEP/REP, etc.
   - ✅ global-data.ll - Initialized arrays, struct access, const data
   - ✅ switch.ll - Switch statements (branch chains)
3. ~~**Code generation audit**~~ - ✅ Done, findings:
   - ✅ AND/OR/XOR DP optimization implemented
   - ✅ Redundant `tax; txa` / `tay; tya` eliminated by peephole pass
   - ⚠️ Signed select has complex branch sequences (inherent to signed compare on 65816)
4. ~~**Clang integration testing**~~ - ✅ Done: C → LLVM IR → W65816 works!
   - Native Clang target: `clang -target w65816-unknown-none -O2`
   - Basic operations work: add, sub, bitwise, shifts, global access
   - Multiply/divide require runtime library (as expected)
5. **Runtime library expansion** - Optional future work
   - Current: __mulhi3, __divhi3, __udivhi3, __modhi3, __umodhi3
   - Potential additions: memcpy, memset, 32-bit math (if needed)
6. ~~**Documentation polish**~~ - ✅ Done: Added C compilation workflow

### Not Planned (Significant Effort)
- **32-bit Integer Support** - Would require full type legalization for i32→i16 pairs, paired register handling for A:X returns, and 32-bit arithmetic via 16-bit pairs. Use library functions or manual 16-bit pairs instead.

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

### Far Functions (Cross-Bank Calls)

Use JSL/RTL for functions in different memory banks:

```c
__attribute__((w65816_farfunc))
int bank1_function(int x) {
    // Called with JSL, returns with RTL
    return x + 1;
}
```

**Note:** Caller and callee must agree - a function with `w65816_farfunc` must always be called with JSL.

### Direct Page Frame Allocation

Use the 256-byte Direct Page region ($00-$FF) for local variables instead of
stack-relative addressing. Provides faster access (2-byte instructions, 1 cycle faster).

```c
// Enable DP frame for a function
__attribute__((annotate("w65816_dpframe")))
int fast_function() {
    int a, b;  // Allocated in DP, not on stack
    // Uses: sta $xx instead of sta n,s
}
```

```bash
# Skip D register save/restore when D is guaranteed to be 0
llc -march=w65816 -mattr=+assume-d0 file.ll
```

**Note:** Locals must fit in 256 bytes or compilation fails with an error.

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

### Compiling C Code

```bash
# Compile C directly to W65816 assembly
./build/bin/clang -target w65816-unknown-none -O2 -S test.c -o test.s

# Compile C to LLVM IR
./build/bin/clang -target w65816-unknown-none -O2 -S -emit-llvm test.c -o test.ll

# Generate object file directly
./build/bin/clang -target w65816-unknown-none -O2 -c test.c -o test.o
```

**Note:** Use `-O2` optimization to avoid alloca-based code that causes register pressure issues.

**Predefined Macros:** The Clang target defines `__W65816__` and `W65816`.
