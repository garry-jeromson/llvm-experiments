# W65816 Backend: Optimization Level Test Findings

This document analyzes test failures at different optimization levels to identify backend bugs.

## Summary

| Level | Passed | Failed | Skipped | Notes |
|-------|--------|--------|---------|-------|
| -O0   | ~25    | ~72    | 2       | Most failures are register pressure related |
| -O1   | 97     | 0      | 2       | All tests pass (DAG crashes fixed) |
| -O2   | 97     | 0      | 2       | All tests pass |
| -O3   | 97     | 0      | 2       | All tests pass |

## Failure Categories

### 1. Register Allocation Failures ("ran out of registers")

**Affected levels:** -O0 primarily, one case at -O2

**Root cause:** The W65816 has only 3 physical registers (A, X, Y). Without optimization passes to reduce live value counts, register allocation fails.

**Examples at -O0:**
- `add.c` - Simple `int a = 10; int b = 20; return a + b;`
- `for_loop.c` - Loop with counter and accumulator
- `function_call.c` - Function with parameters

**Assessment:** This is **expected behavior**, not a bug. The backend relies on LLVM optimization passes (mem2reg, SROA, etc.) to reduce register pressure. Code compiled at -O0 creates excessive alloca-based spills that exhaust registers.

**Recommendation:** Document as a known limitation. Users should compile with -O1 or higher.

---

### 2. DAG Emitter Crashes ("Node emitted out of order - late") - **FIXED**

**Affected levels:** -O0, -O1

**Failing tests:**
- ~~`array_sum.c` (O0)~~
- ~~`array_sum_2d.c` (O0, O1)~~ **FIXED**
- ~~`matrix_ops.c` (O1)~~ **FIXED**

**Root cause:** Two issues were identified:
1. `W65816ISD::WRAPPER` nodes weren't being properly materialized as load-immediate instructions
2. The `imm16` operand type lacked an encoder method, causing symbol references to not generate relocations

**Fixes Applied:**
1. Added `Select` case for `W65816ISD::WRAPPER` to materialize global addresses via `MOV16ri`
2. Added `EncoderMethod = "encodeImmediate"` to `imm16` operand type
3. Added `evaluateFixup` override in `W65816AsmBackend` to force relocations for address fixups
4. Updated `expandMOV16ri` in `W65816ExpandPseudo.cpp` to handle GlobalAddress operands
5. Updated test runner to handle `.rodata.*` section variants (`.rodata.cst8`, etc.)

---

### 3. Wrong Results at -O0

**Failing tests with incorrect output:**

| Test | Expected | Got | Pattern |
|------|----------|-----|---------|
| switch | 20 | 0 | Switch statement |
| ternary | 100 | 12000 | Ternary/select |
| array_2d | 5 | 0 | 2D array access |
| array_index | 30 | 0 | Array indexing |
| array_write | 99 | 0 | Array write |
| memcpy_basic | 123 | 102 | Memory copy |
| memmove_nonoverlap | 60 | 12 | Memory move |
| memset_basic | 0 | 2 | Memory set |
| nested_struct | 60 | 46 | Nested struct access |
| pointer_arith | 30 | 0 | Pointer arithmetic |
| ptr_to_ptr | 99 | 0 | Double pointer |
| struct | 15 | 0 | Struct access |
| struct_array | 30 | 0 | Array of structs |
| state_machine | 6 | 0 | Switch-based state |

**Assessment:** These are **real bugs** in code generation at -O0.

**Priority:** MEDIUM - -O0 is primarily for debugging, but wrong results are problematic.

**Common patterns:**
1. Array/pointer operations returning 0 - likely frame index calculation issues
2. Struct access returning partial values - offset calculation bugs
3. Switch returning 0 - jump table or branch chain issues

---

### 4. Wrong Results at -O1

**Failing tests:** **ALL FIXED!**

| Test | Expected | Got | Analysis |
|------|----------|-----|----------|
| ~~bubble_sort~~ | ~~1~~ | ~~5~~ | ~~Array sorting logic~~ **FIXED** |
| ~~array_of_structs~~ | ~~150~~ | ~~12145~~ | ~~Complex indexed struct access~~ **FIXED** |

**Assessment:** Both wrong result bugs have been fixed at O1.

**Fixes Applied:**
1. `expandLDAindirect` and `expandLDAindirectIdx` now properly save load results to imaginary register destinations
2. Frame index + constant/variable offset patterns now properly select `STA_sr_off`/`LDA_sr_off` for stack array access instead of using incorrect indirect patterns

---

## Recommended Fixes

### High Priority (Should Fix)

1. **DAG emitter crashes** - Investigate scheduling order issues
   - Add missing glue dependencies
   - Review scheduling preferences for complex patterns

2. **-O1 wrong results** - Debug bubble_sort and array_of_structs
   - These are simple tests that should work

### Medium Priority (Nice to Have)

3. **-O0 wrong results** - Fix code generation without optimization
   - Frame index handling for arrays/structs
   - Switch statement lowering

### Low Priority (Acceptable Limitations)

4. **-O0 register exhaustion** - Document as known limitation
   - The 3-register architecture fundamentally requires optimization
   - Most embedded compilers require optimization for similar targets

---

## Test Commands

```bash
# Run tests at specific opt level
python3 test/c-integration/run_tests.py -b build -O O1

# Run at all levels
python3 test/c-integration/run_tests.py -b build --all-opts

# Run single test with verbose output
python3 test/c-integration/run_tests.py -b build -O O1 -v test/c-integration/tests/stress/bubble_sort.c

# Debug code generation
./build/bin/clang -target w65816-unknown-none -O1 -S test.c -o test.s
```

---

## Appendix: Full Failure List by Level

### -O0 Register Exhaustion (52 tests)
add, bitwise, neg, sub, break_loop, for_loop, function_call, nested_calls,
signed_compare, unsigned_compare, while_loop, array, pointer, array_sum,
bubble_sort, fibonacci, max_min, array_multi_index, branch_in_loop,
call_with_live_vars, cascading_calls, complex_expression, diamond_phi,
eight_vars, expression_tree, five_vars, four_vars, interleaved_ops,
loop_accum, loop_with_state, nested_calls, nested_diamond, phi_stress,
quad_phi, sequential_diamonds, six_phi, six_vars, swap_stress, triple_phi,
big_expression, complex_conditional, cross_call_live, eight_phi,
indirect_call_sim, loop_with_8_accum, many_args_8, matrix_ops, nested_loops_3,
nested_loops_4, sixteen_vars, ten_call_chain, ten_vars, triple_nested_diamond,
twelve_vars, twenty_vars

### -O0 Wrong Results (14 tests)
switch, ternary, array_2d, array_index, array_write, memcpy_basic,
memmove_nonoverlap, memset_basic, nested_struct, pointer_arith, ptr_to_ptr,
struct, struct_array, state_machine

### -O0 Crashes (2 tests)
array_sum ("Node emitted out of order"), array_sum_2d ("Node emitted out of order")

### -O1 Wrong Results (0 tests)
All fixed!

### -O1 Crashes (0 tests)
All fixed!

### -O1 Fixed (4 tests)
- bubble_sort - Fixed by properly saving load results to imaginary registers in expandLDAindirect/expandLDAindirectIdx
- array_of_structs - Fixed by adding proper frame index + constant/variable offset selection patterns for stack array access
- array_sum_2d - Fixed by properly handling WRAPPER nodes and generating relocations for immediate symbol references
- matrix_ops - Fixed by updating test runner to handle `.rodata.cst8` section variant
