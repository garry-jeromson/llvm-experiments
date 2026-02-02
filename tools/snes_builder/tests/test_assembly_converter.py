"""Tests for LLVM to ca65 assembly converter."""

import pytest
import tempfile
import os
from pathlib import Path

from tools.snes_builder.assembly_converter import (
    convert_llvm_to_ca65,
    collect_symbols,
    convert_file,
    ConversionResult,
    RUNTIME_SYMBOLS,
)


class TestCollectSymbols:
    """Test symbol collection from assembly."""

    def test_label_defines_symbol(self):
        """Labels define symbols."""
        lines = ["main:", "  rts"]
        defined, referenced = collect_symbols(lines)
        assert "main" in defined

    def test_at_prefix_stripped(self):
        """@ prefix is stripped from labels."""
        lines = ["@main:", "  rts"]
        defined, referenced = collect_symbols(lines)
        assert "main" in defined
        assert "@main" not in defined

    def test_comm_defines_symbol(self):
        """".comm directive defines symbol."""
        lines = [".comm my_var, 2, 2"]
        defined, referenced = collect_symbols(lines)
        assert "my_var" in defined

    def test_local_defines_symbol(self):
        """.local directive defines symbol."""
        lines = [".local my_local"]
        defined, referenced = collect_symbols(lines)
        assert "my_local" in defined

    def test_jsr_references_symbol(self):
        """JSR instruction references symbol."""
        lines = ["jsr _my_func"]
        defined, referenced = collect_symbols(lines)
        assert "_my_func" in referenced

    def test_lda_references_symbol(self):
        """LDA instruction references symbol."""
        lines = ["lda _my_var"]
        defined, referenced = collect_symbols(lines)
        assert "_my_var" in referenced

    def test_offset_stripped(self):
        """Offset is stripped from symbol reference."""
        lines = ["lda _my_var+2"]
        defined, referenced = collect_symbols(lines)
        assert "_my_var" in referenced
        assert "_my_var+2" not in referenced

    def test_numeric_not_symbol(self):
        """Numeric values are not symbols."""
        lines = ["lda $1234", "lda #$FF"]
        defined, referenced = collect_symbols(lines)
        assert "$1234" not in referenced
        assert "#$FF" not in referenced

    def test_underscore_required(self):
        """Symbols must start with underscore."""
        lines = ["lda my_var"]  # No underscore
        defined, referenced = collect_symbols(lines)
        assert "my_var" not in referenced


class TestConvertLlvmToCa65:
    """Test LLVM to ca65 conversion."""

    def test_returns_conversion_result(self):
        """Returns ConversionResult object."""
        result = convert_llvm_to_ca65("")
        assert isinstance(result, ConversionResult)

    def test_header_includes_p816(self):
        """Header includes .p816 directive."""
        result = convert_llvm_to_ca65("")
        assert ".p816" in result.output

    def test_header_includes_smart(self):
        """Header includes .smart directive."""
        result = convert_llvm_to_ca65("")
        assert ".smart" in result.output

    def test_header_includes_a16_i16(self):
        """Header includes 16-bit mode directives."""
        result = convert_llvm_to_ca65("")
        assert ".a16" in result.output
        assert ".i16" in result.output

    def test_runtime_imports_included(self):
        """Runtime library imports are included."""
        result = convert_llvm_to_ca65("")
        for sym in RUNTIME_SYMBOLS:
            assert f".import {sym}" in result.output

    def test_file_directive_skipped(self):
        """.file directive is skipped."""
        result = convert_llvm_to_ca65('.file "test.c"')
        assert '.file' not in result.output

    def test_type_directive_skipped(self):
        """.type directive is skipped."""
        result = convert_llvm_to_ca65('.type main, @function')
        assert '.type' not in result.output

    def test_size_directive_skipped(self):
        """.size directive is skipped."""
        result = convert_llvm_to_ca65('.size main, .Lfunc_end0-main')
        assert '.size' not in result.output

    def test_ident_directive_skipped(self):
        """.ident directive is skipped."""
        result = convert_llvm_to_ca65('.ident "clang version 15"')
        assert '.ident' not in result.output

    def test_cfi_directive_skipped(self):
        """.cfi_* directives are skipped."""
        result = convert_llvm_to_ca65('.cfi_startproc\n.cfi_endproc')
        assert '.cfi_' not in result.output

    def test_p2align_directive_skipped(self):
        """.p2align directive is skipped."""
        result = convert_llvm_to_ca65('.p2align 1')
        assert '.p2align' not in result.output

    def test_local_directive_skipped(self):
        """.local directive is skipped."""
        result = convert_llvm_to_ca65('.local my_local')
        assert '.local' not in result.output

    def test_addrsig_directive_skipped(self):
        """.addrsig directive is skipped."""
        result = convert_llvm_to_ca65('.addrsig')
        assert '.addrsig' not in result.output


class TestSectionConversion:
    """Test section directive conversion."""

    def test_section_text_to_code(self):
        """.section .text converts to .segment "CODE"."""
        result = convert_llvm_to_ca65('.section .text')
        assert '.segment "CODE"' in result.output

    def test_text_to_code(self):
        """.text converts to .segment "CODE"."""
        result = convert_llvm_to_ca65('.text')
        assert '.segment "CODE"' in result.output

    def test_section_rodata_to_rodata(self):
        """.section .rodata converts to .segment "RODATA"."""
        result = convert_llvm_to_ca65('.section .rodata')
        assert '.segment "RODATA"' in result.output

    def test_section_data_to_data(self):
        """.section .data converts to .segment "DATA"."""
        result = convert_llvm_to_ca65('.section .data')
        assert '.segment "DATA"' in result.output

    def test_section_bss_to_bss(self):
        """.section .bss converts to .segment "BSS"."""
        result = convert_llvm_to_ca65('.section .bss')
        assert '.segment "BSS"' in result.output

    def test_section_note_skipped(self):
        """.section .note is skipped."""
        result = convert_llvm_to_ca65('.section .note.GNU-stack')
        assert '.note' not in result.output


class TestGlobalConversion:
    """Test global directive conversion."""

    def test_globl_to_export(self):
        """.globl converts to .export."""
        result = convert_llvm_to_ca65('.globl main')
        assert '.export main' in result.output

    def test_global_to_export(self):
        """.global converts to .export."""
        result = convert_llvm_to_ca65('.global main')
        assert '.export main' in result.output


class TestLabelConversion:
    """Test label conversion."""

    def test_llvm_local_label_converted(self):
        """.LBB labels have dot removed."""
        result = convert_llvm_to_ca65('.LBB0_1:')
        assert 'LBB0_1:' in result.output
        assert '.LBB0_1:' not in result.output

    def test_ltmp_label_converted(self):
        """.Ltmp labels have dot removed."""
        result = convert_llvm_to_ca65('.Ltmp0:')
        assert 'Ltmp0:' in result.output
        assert '.Ltmp0:' not in result.output

    def test_lfunc_end_skipped(self):
        """.Lfunc_end labels are skipped."""
        result = convert_llvm_to_ca65('.Lfunc_end0:')
        assert 'Lfunc_end0' not in result.output

    def test_label_reference_in_branch(self):
        """Label references in branches are converted."""
        result = convert_llvm_to_ca65('bra .LBB0_1')
        assert 'bra LBB0_1' in result.output
        assert '.LBB0_1' not in result.output

    def test_at_prefix_removed(self):
        """@ prefix removed from labels."""
        result = convert_llvm_to_ca65('@main:')
        assert 'main:' in result.output
        assert '@main:' not in result.output


class TestCommConversion:
    """Test .comm directive conversion."""

    def test_comm_to_segment_and_res(self):
        """.comm converts to .segment BSS and .res."""
        result = convert_llvm_to_ca65('.comm my_var, 2, 2')
        assert '.segment "BSS"' in result.output
        assert 'my_var: .res 2' in result.output


class TestZeroConversion:
    """Test .zero directive conversion."""

    def test_zero_to_res(self):
        """.zero converts to .res."""
        result = convert_llvm_to_ca65('.zero 4')
        assert '.res 4' in result.output


class TestExternalSymbolImports:
    """Test external symbol import generation."""

    def test_external_symbol_imported(self):
        """External symbols are imported."""
        asm = """
_my_func:
  rts
main:
  jsr _other_func
  rts
"""
        result = convert_llvm_to_ca65(asm)
        assert '.import _other_func' in result.output
        # _my_func is defined, not imported
        assert '.import _my_func' not in result.output

    def test_defined_symbol_not_imported(self):
        """Defined symbols are not imported."""
        asm = """
_my_func:
  rts
main:
  jsr _my_func
  rts
"""
        result = convert_llvm_to_ca65(asm)
        # _my_func is defined locally
        assert result.output.count('.import _my_func') == 0 or \
               '.import _my_func' not in result.output.split('; External')[0]

    def test_external_symbols_tracked(self):
        """External symbols are tracked in result."""
        asm = """
main:
  jsr _external
  rts
"""
        result = convert_llvm_to_ca65(asm)
        assert '_external' in result.external_symbols


class TestConvertFile:
    """Test file conversion."""

    def test_convert_file_creates_output(self):
        """convert_file creates output file."""
        input_content = """
.text
.globl main
main:
  rts
"""
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "test.s"
            with open(input_path, 'w') as f:
                f.write(input_content)

            output_path = convert_file(input_path)

            assert output_path.exists()
            assert output_path.suffix == '.s'
            assert '.ca65' in output_path.stem

            with open(output_path, 'r') as f:
                output = f.read()
            assert '.p816' in output
            assert '.export main' in output

    def test_convert_file_custom_output(self):
        """convert_file uses custom output path."""
        input_content = ".text"
        with tempfile.TemporaryDirectory() as tmpdir:
            input_path = Path(tmpdir) / "test.s"
            output_path = Path(tmpdir) / "custom.s"

            with open(input_path, 'w') as f:
                f.write(input_content)

            result = convert_file(input_path, output_path)

            assert result == output_path
            assert output_path.exists()


class TestPreservesComments:
    """Test that comments are preserved."""

    def test_semicolon_comment_preserved(self):
        """Semicolon comments are preserved."""
        result = convert_llvm_to_ca65('; This is a comment')
        assert '; This is a comment' in result.output


class TestPreservesInstructions:
    """Test that instructions are preserved."""

    def test_instruction_preserved(self):
        """Instructions are passed through."""
        result = convert_llvm_to_ca65('  lda #$42')
        assert 'lda #$42' in result.output

    def test_indentation_preserved(self):
        """Indentation is preserved."""
        result = convert_llvm_to_ca65('    rts')
        assert '    rts' in result.output


class TestConversionResultFields:
    """Test ConversionResult fields."""

    def test_defined_symbols_populated(self):
        """defined_symbols contains defined symbols."""
        asm = "my_func:\n  rts"
        result = convert_llvm_to_ca65(asm)
        assert "my_func" in result.defined_symbols

    def test_referenced_symbols_populated(self):
        """referenced_symbols contains referenced symbols."""
        asm = "  jsr _other"
        result = convert_llvm_to_ca65(asm)
        assert "_other" in result.referenced_symbols

    def test_external_symbols_computed(self):
        """external_symbols = referenced - defined."""
        asm = """
_my_func:
  jsr _external
  rts
"""
        result = convert_llvm_to_ca65(asm)
        assert "_external" in result.external_symbols
        assert "_my_func" not in result.external_symbols
