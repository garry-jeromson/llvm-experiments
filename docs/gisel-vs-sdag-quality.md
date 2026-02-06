# GlobalISel vs SelectionDAG Code Quality Report

## Executive Summary

GlobalISel achieves full functional parity with SelectionDAG. All 57
integration tests and 121 C integration tests pass with both instruction
selectors. After adding a pre-legalizer combiner pass, G_ZEXTLOAD
legalization, peephole enhancements, immediate-form binary operations,
and multiple peephole bug fixes, the code quality gap has narrowed from
+20.2% to **+7.8%** on integration tests and **+4.9%** on C integration
tests. 83% of C integration tests produce identical code.

### Optimization Pass Impact

| Pass | Integration Gap | C Integration Gap | Notes |
|------|----------------|-------------------|-------|
| Baseline (no optimizations) | +20.2% (711 cycles) | — | — |
| + PreLegalizer Combiner | +14.3% (462 cycles) | +10.2% | 35% gap closed |
| + G_ZEXTLOAD legalization | All tests pass | All tests pass | 5+1 failures fixed |
| + Branch complement peephole | +14.0% | +9.4% | Small |
| + DP load elimination fix | +14.0% (490 cycles) | +9.5% (1,340 cycles) | Improved absolute perf |
| + ULE comparison normalization | +13.5% (472 cycles) | +9.5% | Loop improvement |
| + AND/OR/XOR immediate forms | +10.2% | +6.7% | Avoid materializing constants |
| + Peephole fixes & INY folding | **+7.8% (270 cycles)** | **+4.9% (688 cycles)** | INC_A/INY/STZ now fire |

The combiner includes LLVM's `all_combines` group (50+ rules) plus a custom
`double_neg_fold` rule for integer double-negation identity.

---

## Cycle Count Comparison

### Integration Tests (57 tests, LLVM IR)

All 57 tests pass with both SDAG and GISel.

| Metric | SDAG | GISel | Difference |
|--------|------|-------|------------|
| Total cycles | 3,479 | 3,749 | +270 (+7.8%) |
| Tests with identical cycles | 40/57 (70%) | | |
| Regressions (GISel slower) | | 17 tests | |
| Improvements (GISel faster) | | 0 tests | |

### C Integration Tests (117 common tests, C code)

All 121 tests pass with both SDAG and GISel (2 skipped).

| Metric | SDAG | GISel | Difference |
|--------|------|-------|------------|
| Total cycles | 14,066 | 14,754 | +688 (+4.9%) |
| Tests with identical cycles | 97/117 (83%) | | |
| Regressions (GISel slower) | | 9 tests | |
| Improvements (GISel faster) | | 11 tests | |

---

## Resolved Regressions

### Category 1: Constant Folding (Resolved by Combiner)

The pre-legalizer combiner fully resolves all constant folding regressions.
GISel now generates identical code to SDAG for these tests:

| Test | Before Combiner | After Combiner | Rule |
|------|----------------|----------------|------|
| icmp-slt-zero | 23 insns (+667%) | `lda #3; rts` (identical) | `constant_fold_binops` |
| select | 11 insns (+267%) | `lda #100; rts` (identical) | `select_constant_cmp` |
| double-neg | 17 insns (+183%) | `rts` (identical) | `double_neg_fold` (custom) |
| sext | 7 insns (+133%) | `lda #65408; rts` (identical) | `constant_fold_binops` |

### G_ZEXTLOAD Legalization (Resolved by Custom Lowering)

5 integration tests and 1 C test that previously failed with GISel now pass:
- bool-and, bool-or, bool-xor, byte-add, byte-sub, volatile_u8_compare
- Custom `legalizeExtLoad()` decomposes G_ZEXTLOAD/G_SEXTLOAD into
  G_LOAD + G_ZEXT/G_SEXT since the W65816 has no native extending loads.

### Branch Complement Peephole (Resolved)

The peephole now converts `Bcond .next; BRA .other` into `B!cond .other`
(complemented branch with fall-through). This eliminates one instruction
per occurrence.

### DP Load Elimination Bug Fix

Fixed a bug in `getDPAddress()` that used the wrong operand index for
`LDA_dp`, causing the `STA dp; LDA dp` redundancy elimination to never
fire. This improved both SDAG and GISel across many tests.

### AND/OR/XOR Immediate Forms (Resolved)

Added `AND16ri`, `OR16ri`, `XOR16ri` pseudo instructions that operate
directly with immediate values. Previously GISel would materialize constants
into a register before performing the operation; now it uses `AND #imm` /
`ORA #imm` / `EOR #imm` directly. This closed ~3 percentage points of the
integration gap and ~3 percentage points of the C integration gap.

All `*16ri` pseudo expansions include NeedToSaveA checks using LivePhysRegs
to correctly handle cases where the accumulator holds a live value.

### Peephole Bug Fixes and New Optimizations (Resolved)

Several pre-existing peephole bugs were discovered and fixed:

1. **isAddSubImm1 wrong operand index**: The `CLC; ADC #1 → INC A`
   optimization checked operand 0 for the immediate, but ADC_imm16 has
   the register at operand 0. The optimization never fired. Fixed to scan
   all operands.

2. **isLoadImmZero wrong operand index**: Same bug — the `LDA #0 → STZ`
   optimization never fired. Fixed to scan all operands.

3. **CMP #0 elimination safety**: The optimization removed `CMP #0` after
   `LDA` but didn't check if subsequent branches use V or C flags (which
   LDA does not set). Fixed to scan all following branches and abort if
   any use BVS/BVC/BCS/BCC.

4. **ULE normalization extended**: `CMP #N; BEQ; BCC → CMP #(N+1); BCC`
   now also works for `CPX #N` and `CPY #N` instructions (different
   operand index for the immediate).

5. **Transfer-increment-transfer folding**: New peephole converts
   `TYA; INC A; TAY → INY` (and TXA/DEX/DEY variants) when A is dead
   after the sequence.

6. **INC_A/DEC_A BuildMI fix**: The builder used the wrong constructor
   form, causing assertion failures with tied operands.

Combined with the loop test, these fixes resolved the **loop overhead
category** — the loop integration test now generates identical code
(258 cycles each).

---

## Remaining Regression Analysis

### Category 2: Calling Convention Overhead (Largest Remaining Impact)

When arguments need to be shuffled between registers, GlobalISel generates
extra spills.

| Test | SDAG cycles | GISel cycles | Overhead |
|------|------------|-------------|----------|
| many-args | 184 | 212 | +15.2% |
| call-shuffle-args | 99 | 101 | +2.0% |

**Root cause:** GlobalISel's register allocator doesn't recognize that
the constants can be directly loaded into the target registers without
intermediate shuffling.

**Fix:** Better copy coalescing or a pre-RA combiner that recognizes
direct register assignment patterns.

### Category 3: Byte Extension Overhead (Medium Impact)

GISel's G_ZEXTLOAD decomposition into G_LOAD + G_ZEXT produces extra
instructions compared to SDAG's native pattern matching.

| Test | SDAG cycles | GISel cycles | Overhead |
|------|------------|-------------|----------|
| bool-and | 60 | 74 | +23.3% |
| bool-or | 60 | 74 | +23.3% |
| bool-xor | 60 | 74 | +23.3% |
| byte-add | 59 | 73 | +23.7% |
| byte-sub | 59 | 73 | +23.7% |
| cmp-edge-cases | 53 | 73 | +37.7% |

**Root cause:** SDAG has direct patterns for zero-extending byte loads,
while GISel decomposes them into separate load + extend instructions.

**Fix:** Post-legalizer combiner could fold load + zext back into an
optimized sequence, or instruction selector could pattern-match the pair.

### Category 4: Stack/Memory Access Overhead (Medium Impact)

| Test | SDAG cycles | GISel cycles | Overhead |
|------|------------|-------------|----------|
| stack-struct-addr | 225 | 276 | +22.7% |
| stack-struct-two | 212 | 244 | +15.1% |
| imag-indirect-load | 72 | 101 | +40.3% |
| stack-addr-pass | 114 | 124 | +8.8% |
| stack-struct-simple | 114 | 124 | +8.8% |
| store-load | 39 | 44 | +12.8% |

**Root cause:** Extra register transfers when accessing memory through
pointers or stack-relative addressing.

### Category 5: Complex Algorithm Overhead (C integration only)

A handful of complex C tests drive most of the C integration gap:

| Test | SDAG cycles | GISel cycles | Overhead |
|------|------------|-------------|----------|
| bubble_sort | 2,137 | 2,520 | +383 (+17.9%) |
| state_machine | 478 | 643 | +165 (+34.5%) |
| matrix_ops | 763 | 903 | +140 (+18.3%) |
| switch_many_cases | 413 | 535 | +122 (+29.5%) |

These 4 tests alone account for 810 of the 688 total surplus cycles.
(GISel wins on other tests offset this.) The regressions are due to
register allocation and code scheduling differences in loops and switch
tables.

---

## Where GlobalISel Wins

11 C integration tests show GISel producing better code:

| Test | SDAG cycles | GISel cycles | Improvement |
|------|------------|-------------|-------------|
| volatile_u16_compare | 752 | 695 | -57 (-7.6%) |
| u8_boundary | 569 | 519 | -50 (-8.8%) |
| volatile_u8_all_ops | 902 | 876 | -26 (-2.9%) |
| volatile_i16_ge | 218 | 199 | -19 (-8.7%) |
| volatile_i16_le | 210 | 193 | -17 (-8.1%) |
| u8_compare_const | 400 | 386 | -14 (-3.5%) |
| volatile_i16_gt | 213 | 200 | -13 (-6.1%) |
| volatile_i8_le | 378 | 369 | -9 (-2.4%) |
| volatile_u8_compare | 272 | 255 | -17 (-6.3%) |
| i8_boundary | 467 | 465 | -2 (-0.4%) |
| volatile_i16_slt_zero | 103 | 102 | -1 (-1.0%) |

GlobalISel's advantage is strongest on 8-bit comparison patterns and
volatile access patterns, where its different scheduling decisions
produce tighter code.

---

## Recommendations

### Priority 1: Pre-Legalizer Combiner (Done)
**Done.** The pre-legalizer combiner with `all_combines` + `double_neg_fold`
resolved all Category 1 regressions, closing 35% of the gap.

### Priority 2: G_ZEXTLOAD Legalization (Done)
**Done.** Custom `legalizeExtLoad()` method decomposes extending loads.
All 5+1 previously failing tests now pass.

### Priority 3: Peephole Enhancements (Done)
**Done.** Added peephole optimizations and fixed pre-existing bugs:
- Branch complement: `Bcond .next; BRA .other` → `B!cond .other`
- ULE normalization: `CMP/CPX/CPY #N; BEQ; BCC` → `CMP/CPX/CPY #(N+1); BCC`
- UGE simplification: `BEQ target; BCS target` → `BCS target`
- CMP #0 elimination (with safety check for V/C flag branches)
- Transfer-increment-transfer folding: `TYA; INC A; TAY` → `INY`
- Fixed DP load elimination bug (wrong operand index for `LDA_dp`)
- Fixed `CLC; ADC #1` → `INC A` (never fired due to wrong operand check)
- Fixed `LDA #0; STA` → `STZ` (never fired due to wrong operand check)

### Priority 4: Immediate-Form Binary Operations (Done)
**Done.** Added `AND16ri`, `OR16ri`, `XOR16ri` pseudo instructions.
GISel instruction selector uses immediate forms when RHS is a constant.

### Priority 5: Byte Extension Optimization (Future)
Address Category 3 (byte extension overhead) via:
- Post-legalizer combiner to fold G_LOAD + G_ZEXT patterns
- Or direct pattern matching in instruction selector

Estimated impact: ~14 cycles per test × 6 tests ≈ 84 cycles.

### Priority 6: Register Allocation Improvements (Future)
Address Category 2 (calling convention) and Category 5 (complex algorithms) via:
- Better copy coalescing for argument setup
- Register allocation hints for loop induction variables

---

## Conclusion

GlobalISel achieves functional correctness parity and generates identical
code for **83% of C test cases** (97/117). The optimization work closed
**62% of the integration test gap** (711 → 270 cycles) and achieved a
**4.9% gap on C integration tests** (688 cycles). GISel beats SDAG on 11
C tests. The remaining regressions are concentrated in calling convention
overhead, byte extension patterns, and a handful of complex algorithm tests
— areas that require register allocator improvements and post-legalizer
combining rather than peephole rules.
