"""Tests for GSU register constants and definitions."""

import pytest
from tools.superfx.gsu_registers import (
    # General registers
    R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15,
    # Control registers
    SFR, BRAMR, PBR, ROMBR, CFGR, SCBR, CLSR, SCMR, VCR, RAMBR, CBR,
    # SFR flags
    SFR_Z, SFR_CY, SFR_S, SFR_OV, SFR_G, SFR_R, SFR_ALT1, SFR_ALT2,
    SFR_IL, SFR_IH, SFR_B, SFR_IRQ,
    # SCMR constants
    SCMR_HT_128, SCMR_HT_160, SCMR_HT_192, SCMR_HT_OBJ,
    SCMR_MD_4COLOR, SCMR_MD_16COLOR, SCMR_MD_256COLOR,
    SCMR_RON, SCMR_RAN,
    # Register size
    REGISTER_SIZE,
)


class TestGeneralRegisters:
    """Test general register (R0-R15) addresses."""

    def test_r0_address(self):
        """R0 is at $3000-$3001."""
        assert R0 == 0x3000

    def test_r1_address(self):
        """R1 (PLOT X coordinate) is at $3002-$3003."""
        assert R1 == 0x3002

    def test_r2_address(self):
        """R2 (PLOT Y coordinate) is at $3004-$3005."""
        assert R2 == 0x3004

    def test_r3_through_r13_addresses(self):
        """R3-R13 are at consecutive 2-byte intervals."""
        expected_addresses = [
            (R3, 0x3006),
            (R4, 0x3008),
            (R5, 0x300A),
            (R6, 0x300C),
            (R7, 0x300E),
            (R8, 0x3010),
            (R9, 0x3012),
            (R10, 0x3014),
            (R11, 0x3016),
            (R12, 0x3018),
            (R13, 0x301A),
        ]
        for reg, expected in expected_addresses:
            assert reg == expected

    def test_r14_address(self):
        """R14 (ROM address pointer) is at $301C-$301D."""
        assert R14 == 0x301C

    def test_r15_address(self):
        """R15 (Program Counter) is at $301E-$301F."""
        assert R15 == 0x301E

    def test_register_spacing(self):
        """All registers are spaced 2 bytes apart."""
        registers = [R0, R1, R2, R3, R4, R5, R6, R7, R8, R9, R10, R11, R12, R13, R14, R15]
        for i in range(len(registers) - 1):
            assert registers[i + 1] - registers[i] == 2


class TestControlRegisters:
    """Test control register addresses."""

    def test_sfr_address(self):
        """SFR (Status/Flag Register) is at $3030-$3031."""
        assert SFR == 0x3030

    def test_bramr_address(self):
        """BRAMR (Backup RAM enable) is at $3033."""
        assert BRAMR == 0x3033

    def test_pbr_address(self):
        """PBR (Program Bank Register) is at $3034."""
        assert PBR == 0x3034

    def test_rombr_address(self):
        """ROMBR (ROM Bank Register) is at $3036."""
        assert ROMBR == 0x3036

    def test_cfgr_address(self):
        """CFGR (Config) is at $3037."""
        assert CFGR == 0x3037

    def test_scbr_address(self):
        """SCBR (Screen Base Register) is at $3038."""
        assert SCBR == 0x3038

    def test_clsr_address(self):
        """CLSR (Clock Select) is at $3039."""
        assert CLSR == 0x3039

    def test_scmr_address(self):
        """SCMR (Screen Mode Register) is at $303A."""
        assert SCMR == 0x303A

    def test_vcr_address(self):
        """VCR (Version Code Register) is at $303B."""
        assert VCR == 0x303B

    def test_rambr_address(self):
        """RAMBR (RAM Bank Register) is at $303C."""
        assert RAMBR == 0x303C

    def test_cbr_address(self):
        """CBR (Cache Base Register) is at $303E-$303F."""
        assert CBR == 0x303E


class TestSFRFlags:
    """Test SFR (Status/Flag Register) bit positions."""

    def test_z_flag(self):
        """Z (Zero) flag is bit 1 of low byte."""
        assert SFR_Z == 0x0002

    def test_cy_flag(self):
        """CY (Carry) flag is bit 2 of low byte."""
        assert SFR_CY == 0x0004

    def test_s_flag(self):
        """S (Sign) flag is bit 3 of low byte."""
        assert SFR_S == 0x0008

    def test_ov_flag(self):
        """OV (Overflow) flag is bit 4 of low byte."""
        assert SFR_OV == 0x0010

    def test_g_flag(self):
        """G (Go) flag is bit 5 of low byte."""
        assert SFR_G == 0x0020

    def test_r_flag(self):
        """R (ROM Read) flag is bit 6 of low byte."""
        assert SFR_R == 0x0040

    def test_alt1_flag(self):
        """ALT1 flag is bit 0 of high byte (bit 8)."""
        assert SFR_ALT1 == 0x0100

    def test_alt2_flag(self):
        """ALT2 flag is bit 1 of high byte (bit 9)."""
        assert SFR_ALT2 == 0x0200

    def test_il_flag(self):
        """IL (Immediate Low) flag is bit 2 of high byte (bit 10)."""
        assert SFR_IL == 0x0400

    def test_ih_flag(self):
        """IH (Immediate High) flag is bit 3 of high byte (bit 11)."""
        assert SFR_IH == 0x0800

    def test_b_flag(self):
        """B flag is bit 4 of high byte (bit 12)."""
        assert SFR_B == 0x1000

    def test_irq_flag(self):
        """IRQ flag is bit 7 of high byte (bit 15)."""
        assert SFR_IRQ == 0x8000


class TestSCMRConstants:
    """Test SCMR (Screen Mode Register) constants."""

    def test_screen_height_128(self):
        """128-pixel height mode (HT1:HT0 = 00)."""
        assert SCMR_HT_128 == 0x00

    def test_screen_height_160(self):
        """160-pixel height mode (HT1:HT0 = 01)."""
        assert SCMR_HT_160 == 0x04

    def test_screen_height_192(self):
        """192-pixel height mode (HT1:HT0 = 10)."""
        assert SCMR_HT_192 == 0x20

    def test_screen_height_obj(self):
        """OBJ mode (HT1:HT0 = 11)."""
        assert SCMR_HT_OBJ == 0x24

    def test_color_mode_4(self):
        """4-color mode (MD1:MD0 = 00)."""
        assert SCMR_MD_4COLOR == 0x00

    def test_color_mode_16(self):
        """16-color mode (MD1:MD0 = 01)."""
        assert SCMR_MD_16COLOR == 0x01

    def test_color_mode_256(self):
        """256-color mode (MD1:MD0 = 11)."""
        assert SCMR_MD_256COLOR == 0x03

    def test_ron_flag(self):
        """RON (GSU owns ROM bus) is bit 4."""
        assert SCMR_RON == 0x10

    def test_ran_flag(self):
        """RAN (GSU owns RAM bus) is bit 3."""
        assert SCMR_RAN == 0x08


class TestRegisterSize:
    """Test register size constant."""

    def test_register_size(self):
        """All GSU registers are 16-bit (2 bytes)."""
        assert REGISTER_SIZE == 2
