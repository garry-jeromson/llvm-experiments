# Stack-Struct-Addr Bug Investigation

**STATUS: RESOLVED**

The bug was found in `W65816FrameLowering.cpp` - the epilogue for large stack
frames (StackSize > 8) was broken. See "Root Cause" section below.

## Problem Summary

The `stack-struct-addr` integration test fails when **all** of these conditions are met:
1. Inline initialization of **both** struct members (store directly to stack)
2. Calling **both** `write_first` AND `write_second` functions
3. Reading and summing both members

**Expected:** 30 (first=10, second=20)
**Actual:** 20 (first=0, second=20)

The first member's value gets lost (returns 0 instead of 10).

## Failing Test

See: `test/integration/tests/stack-struct-addr.ll`

```llvm
define i16 @test_main() {
  %s = alloca %struct.pair, align 2

  ; Initialize both members to 0
  store i16 0, ptr %s
  %second = getelementptr inbounds %struct.pair, ptr %s, i16 0, i32 1
  store i16 0, ptr %second

  ; Write 10 to first member via function call
  call void @write_first(ptr %s, i16 10)

  ; Write 20 to second member via function call
  call void @write_second(ptr %s, i16 20)

  ; Read both values and return sum (should be 30)
  %a = load i16, ptr %s
  %b = load i16, ptr %second
  %sum = add i16 %a, %b
  ret i16 %sum
}
```

## What Works (Isolation Tests)

| Test File | Inline Init | Calls | Result |
|-----------|-------------|-------|--------|
| `01-no-inline-init.ll` | No | write_first + write_second | PASS (87) |
| `02-define-both-call-one.ll` | Both members | write_first only | PASS (33) |
| `03-just-init-first.ll` | Both members | write_second only | PASS (11) |
| `stack-struct-simple.ll` | No | write_first only | PASS |
| `stack-struct-two.ll` | No | write_first + write_second | PASS |

The bug **only** manifests when:
- Both struct members are initialized inline (store i16 0)
- AND both write_first and write_second are called

## Investigation Areas

### 1. Code Generation Analysis
- Compare assembly output between working and failing cases
- Focus on code ordering when both inline inits AND both calls are present
- Check if inline init code gets incorrectly hoisted or reordered after function calls

### 2. Register Allocation / Spill Issues
- Examine if LEA_fi for `write_first` clobbers something needed for inline init
- Check if spill/reload sequences are correct across function calls
- Look for registers being inadvertently reused between inline init and call setup

### 3. Frame Index Resolution Ordering
- The combination might expose a timing issue in when frame indices are resolved
- Check if the %second GEP offset is computed correctly in all cases
- Verify that eliminateFrameIndex sees the same offsets for inline init and function calls

### 4. Pseudo Expansion Interactions
- LEA_fi expansion uses TSC + ADC, which clobbers A
- STAindirect adjusts offsets for temporary pushes
- Check if multiple LEA_fi expansions interfere with each other

### 5. MachineIR Dump Analysis
- Run with `-print-after-all` to trace the IR through each pass
- Compare the MachineIR between working and failing cases at each stage
- Identify where the divergence begins

## Debug Commands

```bash
# Generate assembly for failing case
llc -march=w65816 test/integration/tests/stack-struct-addr.ll -o -

# Generate assembly for working case (no inline init)
llc -march=w65816 investigations/stack-struct-addr/01-no-inline-init.ll -o -

# Run with MachineIR debug output
llc -march=w65816 -print-after-all test/integration/tests/stack-struct-addr.ll 2>&1 | less

# Compare assembly between cases
diff <(llc -march=w65816 investigations/stack-struct-addr/01-no-inline-init.ll -o -) \
     <(llc -march=w65816 test/integration/tests/stack-struct-addr.ll -o -)
```

## Key Observations

1. Stack addresses: first member at $01FA, second member at $01FC (offset +2)
2. The 816CE emulator correctly implements STA ($01,s),y addressing
3. Each component (inline init, function calls) works individually
4. Only the specific combination triggers the bug

## Hypotheses

### H1: LEA_fi Clobbers Inline Init Value
The inline `store i16 0, ptr %s` may be getting clobbered by the LEA_fi
expansion for `write_first` if they share a register allocation decision.

### H2: GEP Offset Confusion
The %second GEP is used both for inline init AND for reading the result.
There might be confusion about which offset applies when.

### H3: Instruction Scheduling
The inline init stores might be getting scheduled after the function calls
due to some optimization pass that doesn't account for the calls modifying
the same memory locations.

## Root Cause (FOUND)

The bug was in `W65816FrameLowering.cpp::emitEpilogue()` for functions with
`StackSize > 8`.

The old epilogue code did:
```asm
PHA          ; Push return value (SP -= 2)
TSX          ; Get SP
TXA
CLC
ADC #N+2     ; Add frame size + 2 (for the PHA we just did)
TAX
TXS          ; Set new SP (moved PAST the pushed result!)
PLA          ; Pop - but this pops from wrong location!
```

The problem: After `TXS`, the pushed return value was BELOW the new SP, so
`PLA` popped from the wrong location (the prologue's saved A) instead of the
result we just pushed.

The fix: Use TAY/TYA to save the return value in Y register, avoiding the
push/pop coordination issue:
```asm
TAY          ; Save return value to Y
TSX          ; Get SP
TXA
CLC
ADC #N+2     ; Add frame size + 2 (for prologue PHA)
TAX
TXS          ; Set new SP
TYA          ; Restore return value from Y
```

This matches what the small frame epilogue (StackSize <= 8) already does.

## Files in This Investigation

- `README.md` - This file
- `01-no-inline-init.ll` - No inline init, both calls (PASS)
- `02-define-both-call-one.ll` - Both inits, only write_first called (PASS)
- `03-just-init-first.ll` - Both inits, only write_second called (PASS)
- `04-minimal-both.ll` - Minimal reproducer
- `05-only-inits.ll` - Only inline inits, no function calls
- `06-inits-plus-write-first.ll` - Inits + write_first only
- `07-both-writes-read-first.ll` - Both writes, read only first
- `08-both-writes-read-second.ll` - Both writes, read only second
- `09-read-both-return-first.ll` - Read both, return first
- `10-read-both-add.ll` - Read both and add (the key failing pattern)
- `11-large-frame-no-add.ll` - Large frame, no add
- `12-minimal-large-frame.ll` - Minimal large frame test
- `13-emulator-test.ll` - Simple stack-relative test that isolated the bug
- `failing-case.ll` - Copy of the original failing test
