# W65816 Backend Polish Options

## Low-Hanging Fruit

1. **Fix CMP16rr edge case** - Same bug pattern as SUB16rr (when src1 is X/Y and src2 is A). Not hit in practice due to calling convention, but could cause issues with unusual register allocation.

2. **More peephole optimizations**:
   - `LDA #0` could become `LDA #0` or `TXA` after `LDX #0` (minor)
   - Consecutive `CLC; CLC` or `SEC; SEC` (unlikely but possible)
   - `PHA; PLA` pairs that don't cross branches

3. **Additional integration tests**:
   - Function with many arguments (tests stack passing)
   - Nested function calls
   - Comparison edge cases (INT16_MIN, INT16_MAX)

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
