# W65816 Backend Polish Options

## Completed

1. ~~**Fix CMP16rr edge case**~~ - Verified safe. Unlike SUB16rr, CMP16rr doesn't write back a result (only sets flags), so the defensive code structure is correct.

2. ~~**Peephole optimizations**~~ - All implemented:
   - Consecutive `CLC; CLC` or `SEC; SEC` (duplicate elimination)
   - `SEC; CLC` or `CLC; SEC` (cancellation - first is pointless)
   - `PHA; PLA`, `PHX; PLX`, `PHY; PLY` pairs
   - Redundant transfer pairs (`TAX; TXA`, etc.)
   - `BRA` to fall-through block

3. ~~**Additional integration tests**~~ - All implemented:
   - `many-args.ll` - 6 arguments via stack passing
   - `nested-calls.ll` - Multiple levels of function calls
   - `cmp-edge-cases.ll` - INT16_MIN (-32768) comparison
   - `int16-max-cmp.ll` - INT16_MAX (32767) comparison
   - `double-neg.ll` - Double negation correctness

4. ~~**Instruction selection improvements**~~ - Already implemented:
   - `STZ` (store zero) used for storing zero to absolute addresses
   - `INC abs` / `DEC abs` used for memory increment/decrement

5. ~~**Runtime library expansion**~~ - Implemented:
   - Arithmetic: `__mulhi3`, `__divhi3`, `__udivhi3`, `__modhi3`, `__umodhi3`
   - Memory: `memcpy`, `memset`, `memmove` - useful for struct copies
   - Full test suite (49 tests) verifies all functions
   - Note: 32-bit shift helpers could be added in the future

## Medium Effort

6. **32-bit shift helpers**:
   - For cases where 32-bit operations are needed
   - Could be added to runtime library

7. **Better diagnostics**:
   - Clearer error messages when register pressure exceeds limits
   - Warnings for patterns known to generate suboptimal code

## Larger Efforts (Not Recommended)

8. **32-bit integer support** - Requires type legalization, paired registers. Use library functions instead.

9. **Complex phi node handling** - Deep register allocator changes needed.
