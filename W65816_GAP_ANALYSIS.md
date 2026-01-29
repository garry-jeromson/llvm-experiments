# W65816 LLVM Backend Gap Analysis

Cross-reference of WDC 65816 Programming Manual vs Current Implementation

## Executive Summary

The backend implements approximately **70-75%** of the W65816 capabilities. The main gaps are:
- Long (24-bit) addressing modes
- Block move instructions (MVN/MVP)
- Several indirect addressing variations
- Complete 8-bit mode support
- Some 65816-specific instructions

---

## Addressing Modes: Manual vs Implementation

### ✅ Fully Implemented (13 of 23 modes)

| Mode | Syntax | Status |
|------|--------|--------|
| Implied | `NOP`, `TAX` | ✅ Working |
| Accumulator | `ASL A` | ✅ Working |
| Immediate (16-bit) | `LDA #$1234` | ✅ Working |
| Absolute | `LDA $2000` | ✅ Working |
| Direct Page | `LDA $55` | ✅ Working |
| Absolute Indexed X | `LDA $2000,X` | ✅ Working |
| Absolute Indexed Y | `LDA $2000,Y` | ✅ Working |
| Direct Page Indexed X | `LDA $55,X` | ✅ Working |
| Direct Page Indirect | `LDA ($55)` | ✅ Defined, patterns exist |
| DP Indexed Indirect X | `LDA ($55,X)` | ✅ Defined |
| DP Indirect Indexed Y | `LDA ($55),Y` | ✅ Defined |
| Stack Relative | `LDA 3,S` | ✅ Working (key for spills) |
| Stack Rel Indirect Idx Y | `LDA (5,S),Y` | ✅ Working (for ptr deref) |

### ⚠️ Partially Implemented (2 modes)

| Mode | Syntax | Status |
|------|--------|--------|
| PC Relative (8-bit) | `BEQ label` | ⚠️ Working, but relaxation to 16-bit BRL incomplete |
| Direct Page Indexed Y | `LDX $55,Y` | ⚠️ Only for LDX/STX (per W65816 spec) |

### ❌ Not Implemented (8 modes)

| Mode | Syntax | Description | Priority |
|------|--------|-------------|----------|
| Absolute Indirect | `JMP ($1020)` | Jump through pointer | Medium |
| Absolute Indexed Indirect | `JMP ($2000,X)` | Jump table support | Medium |
| **Absolute Long** | `LDA $02F000` | 24-bit addressing | High |
| **Absolute Long Indexed X** | `LDA $12D080,X` | 24-bit + X | High |
| **DP Indirect Long** | `LDA [$55]` | 24-bit via DP | Medium |
| **DP Indirect Long Idx Y** | `LDA [$55],Y` | 24-bit via DP + Y | Medium |
| **PC Relative Long** | `BRL label` | 16-bit branch offset | Low |
| **Block Move** | `MVP $01,$00` | Memory block copy | Low |

---

## Instructions: Manual vs Implementation

### ✅ Fully Implemented

**Load/Store:** LDA, LDX, LDY, STA, STX, STY, STZ (partial)
**Arithmetic:** ADC, SBC, INC, DEC, INX, INY, DEX, DEY
**Logical:** AND, ORA, EOR
**Shift/Rotate:** ASL, LSR, ROL, ROR (accumulator + all memory modes)
**Compare:** CMP, CPX, CPY
**Branch:** BCC, BCS, BEQ, BNE, BMI, BPL, BVC, BVS, BRA
**Jump:** JMP (absolute), JSR, RTS, RTI
**Transfer:** TAX, TAY, TXA, TYA, TXS, TSX
**Stack:** PHA, PLA, PHX, PLX, PHY, PLY, PHP, PLP
**Flags:** CLC, SEC, CLD, SED, CLI, SEI, CLV
**65816:** REP, SEP, PHD, PLD, PHB, PLB, PHK, RTL, JSL

### ⚠️ Partially Implemented

| Instruction | Issue |
|-------------|-------|
| BIT | ✅ All modes defined (immediate, absolute, DP, abs+X, DP+X) - instructions can be used via assembly |
| ASL/LSR/ROL/ROR | ✅ All modes defined (accumulator, DP, DP+X, abs, abs+X) - no selection patterns yet |
| INC/DEC | ✅ All modes defined (accumulator, absolute, DP, abs+X, DP+X) - memory modes usable via assembly |
| STZ | Missing some addressing modes |

### ❌ Not Implemented

| Instruction | Description | Priority |
|-------------|-------------|----------|
| **MVN** | Block move (increment) | Medium |
| **MVP** | Block move (decrement) | Medium |
| TXY | Transfer X to Y | Low |
| TYX | Transfer Y to X | Low |
| TCS | Transfer C to Stack Ptr | Low |
| TSC | Transfer Stack Ptr to C | Low |
| TCD | Transfer C to Direct Page | Low |
| TDC | Transfer Direct Page to C | Low |
| XBA | Exchange B and A | Low |
| XCE | Exchange Carry/Emulation | Medium (for mode switch) |
| BRL | Branch long (16-bit) | Low |
| PEA | Push Effective Address | Low |
| PEI | Push Effective Indirect | Low |
| PER | Push Effective Relative | Low |
| COP | Co-processor | Low |
| WAI | Wait for Interrupt | Low |
| STP | Stop Processor | Low |
| TRB | Test and Reset Bits | Low |
| TSB | Test and Set Bits | Low |
| BRK | Software Break | Low |

---

## Key Gaps by Priority

### High Priority (Functional Gaps)

1. **Long (24-bit) Addressing**
   - Required for: Cross-bank code/data access, SNES ROM banking
   - Missing: JML, JSL patterns, long load/store patterns
   - Impact: Cannot generate code for multi-bank programs

2. **8-bit Mode Switching**
   - Current: SEP/REP defined but mode tracking incomplete
   - Missing: Dynamic m/x flag state tracking
   - Impact: 8-bit operations work via mode switch pseudo, but not optimal

3. **Memory Shift/Rotate** ✅ Instructions Defined
   - All addressing modes now defined: DP, DP+X, absolute, absolute+X
   - 16 new instructions: ASL/LSR/ROL/ROR × 4 addressing modes
   - Instructions can be used via intrinsics or manual assembly
   - Note: Automatic selection pattern implementation deferred due to DAG chain complexity
   - **Shift type legalization: FIXED** - Changed `getScalarShiftAmountTy()` to return MVT::i16

### Medium Priority (Optimization Opportunities)

4. **Block Move Instructions (MVN/MVP)**
   - Use case: memcpy/memmove for large blocks
   - Impact: 7 cycles/byte vs many more with loop

5. **Indirect Jump Modes**
   - `JMP ($addr)` - Jump through pointer
   - `JMP ($addr,X)` - Jump tables
   - Impact: Function pointers, switch statements

6. **BIT Instruction Full Support**
   - Missing absolute and direct page modes
   - Impact: Bit testing without loading to accumulator

### Low Priority (Nice to Have)

7. **Inter-register Transfers (TXY, TYX)**
8. **16-bit Register Transfers (TCS, TSC, TCD, TDC)**
9. **XBA (byte swap within accumulator)**
10. **Stack push effective address (PEA, PEI, PER)**
11. **Test and modify bits (TRB, TSB)**

---

## Verification Notes

### What's Working Well
- Basic arithmetic and logical operations
- Stack frame management (prologue/epilogue)
- Register spilling via stack-relative
- Indirect pointer dereference through `(offset,S),Y`
- Signed/unsigned comparisons
- ELF object generation

### Areas Needing Testing
- Complex register pressure scenarios
- 32-bit return values (A:X pair)
- Stack-passed arguments beyond 3rd
- Sign extension for 8-bit loads

---

## Recommendations

### Immediate Actions
1. Complete 8-bit mode support (in progress)
2. ~~Add memory shift/rotate instructions~~ ✅ DONE (Jan 2025)
3. Add remaining BIT addressing modes

### Future Enhancements
1. Long addressing for multi-bank support
2. Block move for memcpy optimization
3. Jump table support via indirect jumps

---

## Files to Modify

| File | Changes Needed |
|------|----------------|
| `W65816InstrInfo.td` | Add missing instructions, addressing modes |
| `W65816ExpandPseudo.cpp` | Expand new pseudos |
| `W65816ISelDAGToDAG.cpp` | Pattern matching for new modes |
| `W65816ISelLowering.cpp` | Custom lowering for complex ops |
| `KNOWN_LIMITATIONS.md` | Update with current status |

---

## First Steps: Preserve Reference Materials

1. Copy the converted manual to the working directory:
```bash
cp /private/tmp/claude-502/-Users-garry-jeromson-work-llvm-experiments/32a44564-632a-4e73-9d56-a03144243f21/scratchpad/w65816_manual.txt ./w65816_manual.txt
```

2. Copy this gap analysis plan to the working directory:
```bash
cp /Users/garry.jeromson/.claude/plans/graceful-finding-minsky.md ./W65816_GAP_ANALYSIS.md
```

These reference files will be useful for future sessions:
- `w65816_manual.txt` - 22,000-line WDC 65816 Programming Manual (authoritative reference)
- `W65816_GAP_ANALYSIS.md` - This gap analysis comparing manual to implementation
