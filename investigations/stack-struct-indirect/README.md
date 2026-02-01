# Stack-Struct-Indirect Bug Investigation

**STATUS: RESOLVED**

## Problem Summary

The `stack-struct-addr` and `stack-struct-two` integration tests fail:
- **Expected:** 30 (first=10, second=20)
- **Actual:** 10 (first=10, second=0)

The second member's value (20) is not being written or read correctly.

## Failing Tests

Both tests write to struct members via function calls:
- `write_first(ptr, 10)` - writes 10 to first member
- `write_second(ptr, 20)` - writes 20 to second member (uses GEP +2)

The first member is correctly read (10), but the second member returns 0.

## Key Observations

Looking at the generated assembly for `write_first`:
```asm
write_first:
  pha                   ; Prologue: allocate frame (SP -= 2)
  phx                   ; STAindirect step 1: push value
  sta 3,s               ; STAindirect step 2: store ptr at offset 3
  ldy #0                ; STAindirect step 3: Y = index
  pla                   ; STAindirect step 4: restore value to A
  sta ($01,s),y         ; STAindirect step 5: store through ptr
  tay                   ; Epilogue: save A
  pla                   ; Epilogue: restore frame
  tya                   ; Epilogue: restore A
  rts
```

And for `write_second` (with Y=2 for offset):
```asm
write_second:
  pha                   ; Prologue
  ldy #2                ; Y = 2 (offset to second member)
  phx                   ; STAindirectIdx step 1
  sta 3,s               ; STAindirectIdx step 2
  pla                   ; STAindirectIdx step 4
  sta ($01,s),y         ; STAindirectIdx step 5: store at ptr+2
  tay                   ; Epilogue
  pla                   ; Epilogue
  tya                   ; Epilogue
  rts
```

## Investigation Areas

### 1. STAindirectIdx Expansion
- The `write_second` function uses `STAindirectIdx` which handles Y offset
- Need to verify the offset calculation accounts for prologue push

### 2. Stack Offset Calculation
- After prologue PHA, frame slot is at SP+1
- After STAindirect's PHX, frame slot is at SP+3
- Does the (+2) adjustment in expansion account for both pushes correctly?

### 3. Pointer vs Offset Confusion
- `write_first` uses Y=0 (store at ptr+0)
- `write_second` uses Y=2 (store at ptr+2)
- Is the Y offset being set correctly before the store?

### 4. Test_main Caller Analysis
- How is the struct address computed?
- Is the same address passed to both functions?
- Are there any clobbers between calls?

## Isolation Tests

| Test File | Description | Expected | Status |
|-----------|-------------|----------|--------|
| `01-simple-store.ll` | Simplest indirect store (Y=0) | 42 | PASS |
| `02-write-first-only.ll` | Only call write_first (Y=0) | 10 | PASS |
| `03-write-second-only.ll` | Only call write_second (Y=2) | 20 | PASS (after fix) |
| `04-both-read-first.ll` | Call both, read first | 10 | PASS |
| `05-both-read-second.ll` | Call both, read second | 20 | PASS (after fix) |
| `06-indexed-store-only.ll` | Store at offset 2 | 42 | PASS (after fix) |
| `07-direct-stack-ops.ll` | Direct stack store/load | 42 | PASS |
| `08-direct-second-element.ll` | Direct store to arr[1] | 20 | PASS |
| `09-true-stack-roundtrip.ll` | Store, call, reload | 42 | PASS |

**Key Finding:** The bug was in `LDAindirectIdx` not handling imaginary registers.

## Debug Commands

```bash
# Generate assembly for failing test
./build/bin/llc -march=w65816 test/integration/tests/stack-struct-two.ll -o -

# Run with MachineIR debug output
./build/bin/llc -march=w65816 -print-after-all test/integration/tests/stack-struct-two.ll 2>&1 | less

# Test individual files
./build/bin/llc -march=w65816 investigations/stack-struct-indirect/01-simple-store.ll -o -
```

## Hypotheses

### H1: Y Register Clobbered in write_second
The `ldy #2` happens BEFORE `phx`, but then various operations might clobber Y.

### H2: Stack Offset Wrong for STAindirectIdx
The `STAindirectIdx` expansion might not correctly account for the offset when
the index register is already loaded.

### H3: Prologue/Expansion Interaction
The prologue's PHA and the expansion's PHX might create offset confusion.

## Root Cause

**FOUND AND FIXED**

The issue was in `expandLDAindirectIdx` in `W65816ExpandPseudo.cpp`. This function handles
loading from memory through a pointer with a byte offset (used by GEP with non-zero index).

When the pointer (`PtrReg`) was an **imaginary register** (RS0-RS15, backed by Direct Page),
the expansion code fell through to the `else` branch which assumed `PtrReg == X`. This
generated incorrect code that treated the imaginary register as if it were the X register.

### The Bug (before fix)

```mir
$x = LDAindirectIdx 1, killed $rs0, killed $a
```

The function checked for `PtrReg == Y`, `PtrReg == A`, and then fell through to assume `X`:
```cpp
if (PtrReg == W65816::Y) {
  // ...
} else if (PtrReg == W65816::A) {
  // ...
} else { // PtrReg == X  <-- WRONG: PtrReg could be RS0-RS15!
  // ...
}
```

When PtrReg was RS0 (an imaginary register), this generated code like:
```asm
txa      ; Transfer X to A (but ptr is in RS0, not X!)
sta 1,s  ; Store wrong value to stack slot
```

### The Fix

Added explicit handling for imaginary registers at the start of the function:
```cpp
bool PtrIsImag = W65816::IMAG16RegClass.contains(PtrReg);
bool IdxIsImag = W65816::IMAG16RegClass.contains(IdxReg);

if (PtrIsImag && IdxIsImag) {
  // Both imaginary - load idx to Y, then ptr to A
} else if (PtrIsImag) {
  // Only ptr is imaginary - load from DP
  unsigned PtrDPAddr = getImaginaryRegDPAddr(PtrReg);
  BuildMI(..., W65816::LDA_dp, ...).addImm(PtrDPAddr);
} else if (IdxIsImag) {
  // Only idx is imaginary - load from DP to Y
}
// else: both are physical registers, original logic applies
```

The same fix was applied to `expandSTAindirectIdx` for completeness.

### Verification

After the fix:
- All 43 FileCheck tests pass
- All 53 integration tests pass (including `stack-struct-addr` and `stack-struct-two`)
- The `06-indexed-store-only.ll` isolation test passes

## Files in This Investigation

- `README.md` - This file
- `01-simple-store.ll` - Simplest indirect store test
- More files to be added as investigation progresses
