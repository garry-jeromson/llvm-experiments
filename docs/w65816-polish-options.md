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

## Medium Effort

4. **Runtime library expansion**:
   - `memcpy`, `memset`, `memmove` - useful for struct copies
   - 32-bit shift helpers - for cases where 32-bit ops are needed

5. **Instruction selection improvements**:
   - Use `STZ` (store zero) instead of `LDA #0; STA` where applicable
   - Use `INC abs` / `DEC abs` for memory increment when beneficial

6. **Better diagnostics**:
   - Clearer error messages when register pressure exceeds limits
   - Warnings for patterns known to generate suboptimal code

## Larger Efforts (Not Recommended)

7. **32-bit integer support** - Requires type legalization, paired registers. Use library functions instead.

8. **Complex phi node handling** - Deep register allocator changes needed.
