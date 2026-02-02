"""Tests for GSU assembler."""

import pytest
from tools.superfx.gsu_assembler import GSUAssembler, AssemblyError


class TestSingleInstructionAssembly:
    """Test assembly of individual instructions."""

    def test_assemble_stop(self):
        """Assemble STOP instruction."""
        asm = GSUAssembler()
        code = asm.assemble("STOP")
        assert code == bytes([0x00])

    def test_assemble_nop(self):
        """Assemble NOP instruction."""
        asm = GSUAssembler()
        code = asm.assemble("NOP")
        assert code == bytes([0x01])

    def test_assemble_ibt(self):
        """Assemble IBT instruction with register and immediate."""
        asm = GSUAssembler()
        code = asm.assemble("IBT R1, #42")
        assert code == bytes([0xA1, 0x2A])

    def test_assemble_iwt(self):
        """Assemble IWT instruction with 16-bit immediate."""
        asm = GSUAssembler()
        code = asm.assemble("IWT R0, #$1234")
        assert code == bytes([0xF0, 0x34, 0x12])

    def test_assemble_to(self):
        """Assemble TO instruction."""
        asm = GSUAssembler()
        code = asm.assemble("TO R5")
        assert code == bytes([0x15])

    def test_assemble_from(self):
        """Assemble FROM instruction."""
        asm = GSUAssembler()
        code = asm.assemble("FROM R3")
        assert code == bytes([0xB3])

    def test_assemble_with(self):
        """Assemble WITH instruction."""
        asm = GSUAssembler()
        code = asm.assemble("WITH R7")
        assert code == bytes([0x27])

    def test_assemble_add(self):
        """Assemble ADD instruction."""
        asm = GSUAssembler()
        code = asm.assemble("ADD R5")
        assert code == bytes([0x55])

    def test_assemble_add_immediate(self):
        """Assemble ADD with immediate."""
        asm = GSUAssembler()
        code = asm.assemble("ADD #3")
        assert code == bytes([0x3D, 0x53])

    def test_assemble_sub(self):
        """Assemble SUB instruction."""
        asm = GSUAssembler()
        code = asm.assemble("SUB R10")
        assert code == bytes([0x6A])

    def test_assemble_inc(self):
        """Assemble INC instruction."""
        asm = GSUAssembler()
        code = asm.assemble("INC R2")
        assert code == bytes([0xD2])

    def test_assemble_dec(self):
        """Assemble DEC instruction."""
        asm = GSUAssembler()
        code = asm.assemble("DEC R4")
        assert code == bytes([0xE4])

    def test_assemble_stw(self):
        """Assemble STW instruction."""
        asm = GSUAssembler()
        code = asm.assemble("STW (R5)")
        assert code == bytes([0x35])

    def test_assemble_ldw(self):
        """Assemble LDW instruction."""
        asm = GSUAssembler()
        code = asm.assemble("LDW (R8)")
        assert code == bytes([0x48])

    def test_assemble_plot(self):
        """Assemble PLOT instruction."""
        asm = GSUAssembler()
        code = asm.assemble("PLOT")
        assert code == bytes([0x4C])

    def test_assemble_color(self):
        """Assemble COLOR instruction."""
        asm = GSUAssembler()
        code = asm.assemble("COLOR")
        assert code == bytes([0x4E])

    def test_assemble_mult(self):
        """Assemble MULT instruction."""
        asm = GSUAssembler()
        code = asm.assemble("MULT R6")
        assert code == bytes([0x86])

    def test_assemble_and(self):
        """Assemble AND instruction."""
        asm = GSUAssembler()
        code = asm.assemble("AND R3")
        assert code == bytes([0x73])

    def test_assemble_or(self):
        """Assemble OR instruction."""
        asm = GSUAssembler()
        code = asm.assemble("OR R5")
        assert code == bytes([0xC5])

    def test_assemble_jmp(self):
        """Assemble JMP instruction."""
        asm = GSUAssembler()
        code = asm.assemble("JMP R11")
        assert code == bytes([0x9B])

    def test_assemble_link(self):
        """Assemble LINK instruction."""
        asm = GSUAssembler()
        code = asm.assemble("LINK #2")
        assert code == bytes([0x92])


class TestInstructionSequences:
    """Test assembly of instruction sequences."""

    def test_assemble_multiple_instructions(self):
        """Assemble multiple instructions."""
        asm = GSUAssembler()
        code = asm.assemble("""
            IBT R1, #0
            IBT R2, #0
            COLOR
            PLOT
            STOP
            NOP
        """)
        expected = bytes([
            0xA1, 0x00,  # IBT R1, #0
            0xA2, 0x00,  # IBT R2, #0
            0x4E,        # COLOR
            0x4C,        # PLOT
            0x00,        # STOP
            0x01,        # NOP
        ])
        assert code == expected

    def test_assemble_with_comments(self):
        """Comments should be ignored."""
        asm = GSUAssembler()
        code = asm.assemble("""
            ; This is a comment
            STOP    ; inline comment
            NOP
        """)
        assert code == bytes([0x00, 0x01])

    def test_assemble_empty_lines(self):
        """Empty lines should be ignored."""
        asm = GSUAssembler()
        code = asm.assemble("""

            STOP

            NOP

        """)
        assert code == bytes([0x00, 0x01])


class TestLabelResolution:
    """Test label definition and resolution."""

    def test_branch_forward(self):
        """Forward branch to a label."""
        asm = GSUAssembler()
        code = asm.assemble("""
            BRA skip
            NOP
        skip:
            STOP
            NOP
        """)
        # BRA offset should be +1 (skip over NOP to reach label)
        # BRA is at address 0, label 'skip' is at address 3
        # Offset = target - (branch_addr + 2) = 3 - 2 = 1
        expected = bytes([
            0x05, 0x01,  # BRA +1
            0x01,        # NOP
            0x00,        # STOP
            0x01,        # NOP
        ])
        assert code == expected

    def test_branch_backward(self):
        """Backward branch to a label."""
        asm = GSUAssembler()
        code = asm.assemble("""
        loop:
            INC R1
            BNE loop
            NOP
        """)
        # BNE is at address 1, loop is at address 0
        # Offset = 0 - (1 + 2) = -3
        expected = bytes([
            0xD1,        # INC R1
            0x08, 0xFD,  # BNE -3
            0x01,        # NOP
        ])
        assert code == expected

    def test_multiple_labels(self):
        """Multiple labels in one program."""
        asm = GSUAssembler()
        code = asm.assemble("""
        start:
            INC R1
            BEQ done
            NOP
            BRA start
            NOP
        done:
            STOP
            NOP
        """)
        # INC R1 at 0, BEQ at 1, NOP at 3, BRA at 4, NOP at 6, STOP at 7
        # BEQ target 'done' at 7: offset = 7 - (1 + 2) = 4
        # BRA target 'start' at 0: offset = 0 - (4 + 2) = -6
        expected = bytes([
            0xD1,        # INC R1 (addr 0)
            0x09, 0x04,  # BEQ +4 (addr 1-2)
            0x01,        # NOP (addr 3)
            0x05, 0xFA,  # BRA -6 (addr 4-5)
            0x01,        # NOP (addr 6)
            0x00,        # STOP (addr 7)
            0x01,        # NOP (addr 8)
        ])
        assert code == expected


class TestHexAndBinaryLiterals:
    """Test hex and binary literal parsing."""

    def test_hex_dollar_prefix(self):
        """$xx style hex literals."""
        asm = GSUAssembler()
        code = asm.assemble("IBT R0, #$FF")
        assert code == bytes([0xA0, 0xFF])

    def test_hex_0x_prefix(self):
        """0x style hex literals."""
        asm = GSUAssembler()
        code = asm.assemble("IBT R0, #0xFF")
        assert code == bytes([0xA0, 0xFF])

    def test_binary_literal(self):
        """% style binary literals."""
        asm = GSUAssembler()
        code = asm.assemble("IBT R0, #%11110000")
        assert code == bytes([0xA0, 0xF0])

    def test_hex_16bit(self):
        """16-bit hex literal."""
        asm = GSUAssembler()
        code = asm.assemble("IWT R0, #$ABCD")
        assert code == bytes([0xF0, 0xCD, 0xAB])


class TestCaseInsensitivity:
    """Test case insensitivity of mnemonics and registers."""

    def test_lowercase_mnemonic(self):
        """Lowercase mnemonics should work."""
        asm = GSUAssembler()
        code = asm.assemble("stop")
        assert code == bytes([0x00])

    def test_mixed_case_mnemonic(self):
        """Mixed case mnemonics should work."""
        asm = GSUAssembler()
        code = asm.assemble("Stop")
        assert code == bytes([0x00])

    def test_lowercase_register(self):
        """Lowercase registers should work."""
        asm = GSUAssembler()
        code = asm.assemble("ibt r1, #0")
        assert code == bytes([0xA1, 0x00])


class TestErrorHandling:
    """Test error handling for invalid input."""

    def test_unknown_instruction(self):
        """Unknown instruction raises AssemblyError."""
        asm = GSUAssembler()
        with pytest.raises(AssemblyError, match="Unknown instruction"):
            asm.assemble("INVALID")

    def test_undefined_label(self):
        """Undefined label raises AssemblyError."""
        asm = GSUAssembler()
        with pytest.raises(AssemblyError, match="Undefined label"):
            asm.assemble("BRA undefined_label")

    def test_missing_operand(self):
        """Missing operand raises AssemblyError."""
        asm = GSUAssembler()
        with pytest.raises(AssemblyError):
            asm.assemble("IBT")

    def test_invalid_register_number(self):
        """Invalid register number raises AssemblyError."""
        asm = GSUAssembler()
        with pytest.raises(AssemblyError):
            asm.assemble("IBT R16, #0")

    def test_branch_out_of_range(self):
        """Branch out of range raises AssemblyError."""
        asm = GSUAssembler()
        # Create a program with a branch that's too far
        lines = ["start:"] + ["NOP"] * 130 + ["BRA start"]
        with pytest.raises(AssemblyError, match="out of range"):
            asm.assemble("\n".join(lines))


class TestOriginDirective:
    """Test .org directive for setting base address."""

    def test_org_affects_label_addresses(self):
        """Origin directive sets base address."""
        asm = GSUAssembler()
        code = asm.assemble("""
            .org $8000
        start:
            BRA skip
            NOP
        skip:
            STOP
            NOP
        """)
        # Same code as without .org - offset calculation is relative
        expected = bytes([
            0x05, 0x01,  # BRA +1
            0x01,        # NOP
            0x00,        # STOP
            0x01,        # NOP
        ])
        assert code == expected

    def test_get_symbol_address(self):
        """Can retrieve symbol addresses after assembly."""
        asm = GSUAssembler()
        asm.assemble("""
            .org $8000
        start:
            NOP
        end:
            STOP
        """)
        assert asm.get_symbol("start") == 0x8000
        assert asm.get_symbol("end") == 0x8001


class TestMemoryInstructions:
    """Test memory instruction assembly."""

    def test_lms(self):
        """Assemble LMS instruction."""
        asm = GSUAssembler()
        code = asm.assemble("LMS R0, ($20)")
        assert code == bytes([0x3D, 0xA0, 0x20])

    def test_sms(self):
        """Assemble SMS instruction."""
        asm = GSUAssembler()
        code = asm.assemble("SMS ($30), R5")
        assert code == bytes([0x3E, 0xA5, 0x30])

    def test_lm(self):
        """Assemble LM instruction."""
        asm = GSUAssembler()
        code = asm.assemble("LM R0, ($1234)")
        assert code == bytes([0x3D, 0xF0, 0x34, 0x12])

    def test_sm(self):
        """Assemble SM instruction."""
        asm = GSUAssembler()
        code = asm.assemble("SM ($ABCD), R8")
        assert code == bytes([0x3E, 0xF8, 0xCD, 0xAB])


class TestCompletePrograms:
    """Test assembly of complete programs from the reference."""

    def test_fill_screen_program(self):
        """Assemble the fill screen example from docs."""
        asm = GSUAssembler()
        code = asm.assemble("""
            ; GSU code to fill screen with a color
            IBT R1, #0      ; X = 0
            IBT R2, #0      ; Y = 0
            IBT R0, #1      ; Color = 1
            COLOR           ; Set color register

        fill_loop:
            PLOT            ; Plot pixel at (R1, R2)
            INC R1          ; X++
            IBT R3, #$FF    ; Compare value (255)
            WITH R1
            CMP R3          ; R1 == 255?
            BNE fill_loop   ; Continue row
            NOP             ; Delay slot

            IBT R1, #0      ; X = 0
            INC R2          ; Y++
            IBT R3, #127    ; Compare value
            WITH R2
            CMP R3          ; R2 == 127?
            BNE fill_loop   ; Continue screen
            NOP             ; Delay slot

            STOP            ; Done
            NOP             ; Required after STOP
        """)
        # Just verify it assembles and has reasonable length
        assert len(code) > 20
        assert code[0] == 0xA1  # IBT R1
        assert code[-2:] == bytes([0x00, 0x01])  # STOP, NOP


class TestAssembleInstruction:
    """Test single instruction assembly method."""

    def test_assemble_single_instruction(self):
        """assemble_instruction returns bytes for one instruction."""
        asm = GSUAssembler()
        code = asm.assemble_instruction("IBT R5, #$42")
        assert code == bytes([0xA5, 0x42])

    def test_assemble_instruction_strips_comments(self):
        """Comments are stripped from single instruction."""
        asm = GSUAssembler()
        code = asm.assemble_instruction("NOP ; this is a comment")
        assert code == bytes([0x01])
