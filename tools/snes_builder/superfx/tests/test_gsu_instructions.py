"""Tests for GSU instruction encoding."""

import pytest
from tools.snes_builder.superfx.gsu_instructions import (
    # Control instructions
    STOP, NOP, CACHE, LOOP,
    # Shift instructions
    LSR, ROL, ASR, ROR, DIV2,
    # Branch instructions
    BRA, BGE, BLT, BNE, BEQ, BPL, BMI, BCC, BCS, BVC, BVS,
    # Register selection prefixes
    TO, FROM, WITH, ALT1, ALT2, ALT3,
    # Memory instructions
    STW, LDW, STB, LDB, SBK,
    LMS, SMS, LM, SM,
    # Plotting instructions
    PLOT, RPIX, COLOR, CMODE,
    # Byte instructions
    SWAP, NOT, HIB, LOB, SEX, MERGE,
    # Arithmetic instructions
    ADD, SUB, ADC, SBC, CMP, MULT, UMULT, FMULT, LMULT, INC, DEC,
    # Logical instructions
    AND, OR, XOR, BIC,
    # Immediate load instructions
    IBT, IWT,
    # ROM access instructions
    GETB, GETBH, GETBL, GETBS, GETC, ROMB, RAMB,
    # Jump instructions
    JMP, LJMP, LINK,
    # Instruction encoding helper
    encode_instruction,
)


class TestControlInstructions:
    """Test control instruction encoding."""

    def test_stop(self):
        """STOP is opcode $00."""
        assert STOP().encode() == bytes([0x00])

    def test_nop(self):
        """NOP is opcode $01."""
        assert NOP().encode() == bytes([0x01])

    def test_cache(self):
        """CACHE is opcode $02."""
        assert CACHE().encode() == bytes([0x02])

    def test_loop(self):
        """LOOP is opcode $3C."""
        assert LOOP().encode() == bytes([0x3C])


class TestShiftInstructions:
    """Test shift instruction encoding."""

    def test_lsr(self):
        """LSR is opcode $03."""
        assert LSR().encode() == bytes([0x03])

    def test_rol(self):
        """ROL is opcode $04."""
        assert ROL().encode() == bytes([0x04])

    def test_asr(self):
        """ASR is opcode $96."""
        assert ASR().encode() == bytes([0x96])

    def test_ror(self):
        """ROR is opcode $97."""
        assert ROR().encode() == bytes([0x97])

    def test_div2(self):
        """DIV2 is ALT1 + ASR (opcode $3D $96)."""
        assert DIV2().encode() == bytes([0x3D, 0x96])


class TestBranchInstructions:
    """Test branch instruction encoding."""

    def test_bra_positive(self):
        """BRA with positive offset."""
        # BRA +10 should be opcode $05 followed by offset 10
        assert BRA(10).encode() == bytes([0x05, 0x0A])

    def test_bra_negative(self):
        """BRA with negative offset (two's complement)."""
        # BRA -10 should be opcode $05 followed by offset 0xF6 (-10 in two's complement)
        assert BRA(-10).encode() == bytes([0x05, 0xF6])

    def test_bge(self):
        """BGE is opcode $06."""
        assert BGE(5).encode() == bytes([0x06, 0x05])

    def test_blt(self):
        """BLT is opcode $07."""
        assert BLT(5).encode() == bytes([0x07, 0x05])

    def test_bne(self):
        """BNE is opcode $08."""
        assert BNE(5).encode() == bytes([0x08, 0x05])

    def test_beq(self):
        """BEQ is opcode $09."""
        assert BEQ(5).encode() == bytes([0x09, 0x05])

    def test_bpl(self):
        """BPL is opcode $0A."""
        assert BPL(5).encode() == bytes([0x0A, 0x05])

    def test_bmi(self):
        """BMI is opcode $0B."""
        assert BMI(5).encode() == bytes([0x0B, 0x05])

    def test_bcc(self):
        """BCC is opcode $0C."""
        assert BCC(5).encode() == bytes([0x0C, 0x05])

    def test_bcs(self):
        """BCS is opcode $0D."""
        assert BCS(5).encode() == bytes([0x0D, 0x05])

    def test_bvc(self):
        """BVC is opcode $0E."""
        assert BVC(5).encode() == bytes([0x0E, 0x05])

    def test_bvs(self):
        """BVS is opcode $0F."""
        assert BVS(5).encode() == bytes([0x0F, 0x05])


class TestRegisterPrefixes:
    """Test register selection prefix encoding."""

    def test_to_r0(self):
        """TO R0 is opcode $10."""
        assert TO(0).encode() == bytes([0x10])

    def test_to_r15(self):
        """TO R15 is opcode $1F."""
        assert TO(15).encode() == bytes([0x1F])

    def test_with_r0(self):
        """WITH R0 is opcode $20."""
        assert WITH(0).encode() == bytes([0x20])

    def test_with_r15(self):
        """WITH R15 is opcode $2F."""
        assert WITH(15).encode() == bytes([0x2F])

    def test_from_r0(self):
        """FROM R0 is opcode $B0."""
        assert FROM(0).encode() == bytes([0xB0])

    def test_from_r15(self):
        """FROM R15 is opcode $BF."""
        assert FROM(15).encode() == bytes([0xBF])

    def test_alt1(self):
        """ALT1 is opcode $3D."""
        assert ALT1().encode() == bytes([0x3D])

    def test_alt2(self):
        """ALT2 is opcode $3E."""
        assert ALT2().encode() == bytes([0x3E])

    def test_alt3(self):
        """ALT3 is opcode $3F."""
        assert ALT3().encode() == bytes([0x3F])


class TestMemoryInstructions:
    """Test memory instruction encoding."""

    def test_stw_r0(self):
        """STW (R0) is opcode $30."""
        assert STW(0).encode() == bytes([0x30])

    def test_stw_r11(self):
        """STW (R11) is opcode $3B."""
        assert STW(11).encode() == bytes([0x3B])

    def test_ldw_r0(self):
        """LDW (R0) is opcode $40."""
        assert LDW(0).encode() == bytes([0x40])

    def test_ldw_r11(self):
        """LDW (R11) is opcode $4B."""
        assert LDW(11).encode() == bytes([0x4B])

    def test_stb_r0(self):
        """STB (R0) is ALT1 + STW = $3D $30."""
        assert STB(0).encode() == bytes([0x3D, 0x30])

    def test_ldb_r0(self):
        """LDB (R0) is ALT1 + LDW = $3D $40."""
        assert LDB(0).encode() == bytes([0x3D, 0x40])

    def test_sbk(self):
        """SBK is opcode $90."""
        assert SBK().encode() == bytes([0x90])

    def test_lms_r0(self):
        """LMS R0, (xx) is ALT1 + IBT = $3D $A0 xx."""
        assert LMS(0, 0x20).encode() == bytes([0x3D, 0xA0, 0x20])

    def test_sms_r0(self):
        """SMS (xx), R0 is ALT2 + IBT = $3E $A0 xx."""
        assert SMS(0, 0x20).encode() == bytes([0x3E, 0xA0, 0x20])

    def test_lm_r0(self):
        """LM R0, (xxxx) is ALT1 + IWT = $3D $F0 xx xx."""
        assert LM(0, 0x1234).encode() == bytes([0x3D, 0xF0, 0x34, 0x12])

    def test_sm_r0(self):
        """SM (xxxx), R0 is ALT2 + IWT = $3E $F0 xx xx."""
        assert SM(0, 0x1234).encode() == bytes([0x3E, 0xF0, 0x34, 0x12])


class TestPlottingInstructions:
    """Test plotting instruction encoding."""

    def test_plot(self):
        """PLOT is opcode $4C."""
        assert PLOT().encode() == bytes([0x4C])

    def test_rpix(self):
        """RPIX is ALT1 + PLOT = $3D $4C."""
        assert RPIX().encode() == bytes([0x3D, 0x4C])

    def test_color(self):
        """COLOR is opcode $4E."""
        assert COLOR().encode() == bytes([0x4E])

    def test_cmode(self):
        """CMODE is ALT1 + COLOR = $3D $4E."""
        assert CMODE().encode() == bytes([0x3D, 0x4E])


class TestByteInstructions:
    """Test byte manipulation instruction encoding."""

    def test_swap(self):
        """SWAP is opcode $4D."""
        assert SWAP().encode() == bytes([0x4D])

    def test_not(self):
        """NOT is opcode $4F."""
        assert NOT().encode() == bytes([0x4F])

    def test_hib(self):
        """HIB is opcode $C0."""
        assert HIB().encode() == bytes([0xC0])

    def test_lob(self):
        """LOB is opcode $9E."""
        assert LOB().encode() == bytes([0x9E])

    def test_sex(self):
        """SEX is opcode $95."""
        assert SEX().encode() == bytes([0x95])

    def test_merge(self):
        """MERGE is opcode $70."""
        assert MERGE().encode() == bytes([0x70])


class TestArithmeticInstructions:
    """Test arithmetic instruction encoding."""

    def test_add_r0(self):
        """ADD R0 is opcode $50."""
        assert ADD(0).encode() == bytes([0x50])

    def test_add_r15(self):
        """ADD R15 is opcode $5F."""
        assert ADD(15).encode() == bytes([0x5F])

    def test_add_immediate(self):
        """ADD #n is ALT1 + ADD = $3D $5n."""
        assert ADD(5, immediate=True).encode() == bytes([0x3D, 0x55])

    def test_sub_r0(self):
        """SUB R0 is opcode $60."""
        assert SUB(0).encode() == bytes([0x60])

    def test_sub_r15(self):
        """SUB R15 is opcode $6F."""
        assert SUB(15).encode() == bytes([0x6F])

    def test_sub_immediate(self):
        """SUB #n is ALT1 + SUB = $3D $6n."""
        assert SUB(5, immediate=True).encode() == bytes([0x3D, 0x65])

    def test_adc_r0(self):
        """ADC R0 is ALT2 + ADD = $3E $50."""
        assert ADC(0).encode() == bytes([0x3E, 0x50])

    def test_sbc_r0(self):
        """SBC R0 is ALT2 + SUB = $3E $60."""
        assert SBC(0).encode() == bytes([0x3E, 0x60])

    def test_cmp_r0(self):
        """CMP R0 is ALT3 + SUB = $3F $60."""
        assert CMP(0).encode() == bytes([0x3F, 0x60])

    def test_mult_r0(self):
        """MULT R0 is opcode $80."""
        assert MULT(0).encode() == bytes([0x80])

    def test_mult_immediate(self):
        """MULT #n is ALT1 + MULT = $3D $8n."""
        assert MULT(5, immediate=True).encode() == bytes([0x3D, 0x85])

    def test_umult_r0(self):
        """UMULT R0 is ALT2 + MULT = $3E $80."""
        assert UMULT(0).encode() == bytes([0x3E, 0x80])

    def test_umult_immediate(self):
        """UMULT #n is ALT3 + MULT = $3F $8n."""
        assert UMULT(5, immediate=True).encode() == bytes([0x3F, 0x85])

    def test_fmult(self):
        """FMULT is opcode $9F."""
        assert FMULT().encode() == bytes([0x9F])

    def test_lmult(self):
        """LMULT is ALT1 + FMULT = $3D $9F."""
        assert LMULT().encode() == bytes([0x3D, 0x9F])

    def test_inc_r0(self):
        """INC R0 is opcode $D0."""
        assert INC(0).encode() == bytes([0xD0])

    def test_inc_r14(self):
        """INC R14 is opcode $DE."""
        assert INC(14).encode() == bytes([0xDE])

    def test_dec_r0(self):
        """DEC R0 is opcode $E0."""
        assert DEC(0).encode() == bytes([0xE0])

    def test_dec_r14(self):
        """DEC R14 is opcode $EE."""
        assert DEC(14).encode() == bytes([0xEE])


class TestLogicalInstructions:
    """Test logical instruction encoding."""

    def test_and_r1(self):
        """AND R1 is opcode $71."""
        assert AND(1).encode() == bytes([0x71])

    def test_and_r15(self):
        """AND R15 is opcode $7F."""
        assert AND(15).encode() == bytes([0x7F])

    def test_and_immediate(self):
        """AND #n is ALT1 + AND = $3D $7n."""
        assert AND(5, immediate=True).encode() == bytes([0x3D, 0x75])

    def test_or_r1(self):
        """OR R1 is opcode $C1."""
        assert OR(1).encode() == bytes([0xC1])

    def test_or_r15(self):
        """OR R15 is opcode $CF."""
        assert OR(15).encode() == bytes([0xCF])

    def test_or_immediate(self):
        """OR #n is ALT1 + OR = $3D $Cn."""
        assert OR(5, immediate=True).encode() == bytes([0x3D, 0xC5])

    def test_xor_r1(self):
        """XOR R1 is ALT2 + OR = $3E $C1."""
        assert XOR(1).encode() == bytes([0x3E, 0xC1])

    def test_xor_immediate(self):
        """XOR #n is ALT3 + OR = $3F $Cn."""
        assert XOR(5, immediate=True).encode() == bytes([0x3F, 0xC5])

    def test_bic_r1(self):
        """BIC R1 is ALT2 + AND = $3E $71."""
        assert BIC(1).encode() == bytes([0x3E, 0x71])

    def test_bic_immediate(self):
        """BIC #n is ALT3 + AND = $3F $7n."""
        assert BIC(5, immediate=True).encode() == bytes([0x3F, 0x75])


class TestImmediateLoadInstructions:
    """Test immediate load instruction encoding."""

    def test_ibt_r0_positive(self):
        """IBT R0, #xx with positive value."""
        assert IBT(0, 0x12).encode() == bytes([0xA0, 0x12])

    def test_ibt_r15_positive(self):
        """IBT R15, #xx with positive value."""
        assert IBT(15, 0x12).encode() == bytes([0xAF, 0x12])

    def test_ibt_r0_negative(self):
        """IBT R0, #xx with negative value (sign extended)."""
        # -10 in signed byte is 0xF6
        assert IBT(0, -10).encode() == bytes([0xA0, 0xF6])

    def test_iwt_r0(self):
        """IWT R0, #xxxx loads 16-bit immediate."""
        assert IWT(0, 0x1234).encode() == bytes([0xF0, 0x34, 0x12])

    def test_iwt_r15(self):
        """IWT R15, #xxxx loads 16-bit immediate."""
        assert IWT(15, 0x1234).encode() == bytes([0xFF, 0x34, 0x12])


class TestROMAccessInstructions:
    """Test ROM access instruction encoding."""

    def test_getb(self):
        """GETB is opcode $EF."""
        assert GETB().encode() == bytes([0xEF])

    def test_getbh(self):
        """GETBH is ALT1 + GETB = $3D $EF."""
        assert GETBH().encode() == bytes([0x3D, 0xEF])

    def test_getbl(self):
        """GETBL is ALT2 + GETB = $3E $EF."""
        assert GETBL().encode() == bytes([0x3E, 0xEF])

    def test_getbs(self):
        """GETBS is ALT3 + GETB = $3F $EF."""
        assert GETBS().encode() == bytes([0x3F, 0xEF])

    def test_getc(self):
        """GETC is opcode $DF."""
        assert GETC().encode() == bytes([0xDF])

    def test_romb(self):
        """ROMB is ALT2 + GETC = $3E $DF."""
        assert ROMB().encode() == bytes([0x3E, 0xDF])

    def test_ramb(self):
        """RAMB is ALT1 + GETC = $3D $DF."""
        assert RAMB().encode() == bytes([0x3D, 0xDF])


class TestJumpInstructions:
    """Test jump instruction encoding."""

    def test_jmp_r8(self):
        """JMP R8 is opcode $98."""
        assert JMP(8).encode() == bytes([0x98])

    def test_jmp_r13(self):
        """JMP R13 is opcode $9D."""
        assert JMP(13).encode() == bytes([0x9D])

    def test_ljmp_r8(self):
        """LJMP R8 is ALT1 + JMP = $3D $98."""
        assert LJMP(8).encode() == bytes([0x3D, 0x98])

    def test_link_1(self):
        """LINK #1 is opcode $91."""
        assert LINK(1).encode() == bytes([0x91])

    def test_link_4(self):
        """LINK #4 is opcode $94."""
        assert LINK(4).encode() == bytes([0x94])


class TestInvalidInputs:
    """Test error handling for invalid inputs."""

    def test_invalid_register_to(self):
        """TO with register > 15 raises ValueError."""
        with pytest.raises(ValueError):
            TO(16)

    def test_invalid_register_negative(self):
        """TO with negative register raises ValueError."""
        with pytest.raises(ValueError):
            TO(-1)

    def test_stw_invalid_register(self):
        """STW with register > 11 raises ValueError."""
        with pytest.raises(ValueError):
            STW(12)

    def test_ldw_invalid_register(self):
        """LDW with register > 11 raises ValueError."""
        with pytest.raises(ValueError):
            LDW(12)

    def test_jmp_invalid_register(self):
        """JMP with register < 8 raises ValueError."""
        with pytest.raises(ValueError):
            JMP(7)

    def test_jmp_invalid_register_high(self):
        """JMP with register > 13 raises ValueError."""
        with pytest.raises(ValueError):
            JMP(14)

    def test_link_invalid_value_low(self):
        """LINK with n < 1 raises ValueError."""
        with pytest.raises(ValueError):
            LINK(0)

    def test_link_invalid_value_high(self):
        """LINK with n > 4 raises ValueError."""
        with pytest.raises(ValueError):
            LINK(5)

    def test_inc_invalid_register(self):
        """INC R15 is not valid (R15 is PC)."""
        with pytest.raises(ValueError):
            INC(15)

    def test_dec_invalid_register(self):
        """DEC R15 is not valid (R15 is PC)."""
        with pytest.raises(ValueError):
            DEC(15)

    def test_branch_offset_too_large(self):
        """Branch with offset > 127 raises ValueError."""
        with pytest.raises(ValueError):
            BRA(128)

    def test_branch_offset_too_small(self):
        """Branch with offset < -128 raises ValueError."""
        with pytest.raises(ValueError):
            BRA(-129)


class TestEncodeInstructionHelper:
    """Test the encode_instruction helper function."""

    def test_encode_stop(self):
        """encode_instruction('STOP') returns correct bytes."""
        assert encode_instruction('STOP') == bytes([0x00])

    def test_encode_nop(self):
        """encode_instruction('NOP') returns correct bytes."""
        assert encode_instruction('NOP') == bytes([0x01])

    def test_encode_ibt(self):
        """encode_instruction('IBT', 1, 42) returns correct bytes."""
        assert encode_instruction('IBT', 1, 42) == bytes([0xA1, 0x2A])

    def test_encode_iwt(self):
        """encode_instruction('IWT', 0, 0x1234) returns correct bytes."""
        assert encode_instruction('IWT', 0, 0x1234) == bytes([0xF0, 0x34, 0x12])
