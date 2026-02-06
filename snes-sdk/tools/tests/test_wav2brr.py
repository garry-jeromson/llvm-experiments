#!/usr/bin/env python3
"""
Unit tests for wav2brr.py
"""

import sys
import unittest
import struct
import tempfile
from pathlib import Path

# Add parent directory to path
sys.path.insert(0, str(Path(__file__).parent.parent))

# Import module under test
from wav2brr import (
    read_wav,
    resample,
    encode_brr_block,
    encode_brr,
    format_assembly,
)


class TestResampling(unittest.TestCase):
    """Test audio resampling."""

    def test_no_resample(self):
        """Same rate should return same samples."""
        samples = [0, 100, 200, 300, 400]
        result = resample(samples, 16000, 16000)
        self.assertEqual(result, samples)

    def test_downsample_half(self):
        """Downsampling by 2x should halve sample count."""
        samples = list(range(100))
        result = resample(samples, 32000, 16000)
        self.assertEqual(len(result), 50)

    def test_upsample_double(self):
        """Upsampling by 2x should double sample count."""
        samples = list(range(100))
        result = resample(samples, 16000, 32000)
        self.assertEqual(len(result), 200)

    def test_linear_interpolation(self):
        """Upsampling should interpolate linearly."""
        samples = [0, 100]
        result = resample(samples, 10, 20)
        # Should have intermediate value
        self.assertEqual(len(result), 4)
        # First and last should be close to original
        self.assertEqual(result[0], 0)
        # Middle values should be interpolated
        self.assertTrue(0 < result[1] < 100)


class TestBRREncoding(unittest.TestCase):
    """Test BRR encoding."""

    def test_block_size(self):
        """BRR block should be 9 bytes."""
        samples = [0] * 16
        block, _ = encode_brr_block(samples, (0, 0))
        self.assertEqual(len(block), 9)

    def test_header_format(self):
        """BRR header should have correct format."""
        samples = [0] * 16
        block, _ = encode_brr_block(samples, (0, 0))
        header = block[0]
        # Range is in upper nibble, filter in bits 2-3
        range_val = (header >> 4) & 0x0F
        filter_val = (header >> 2) & 0x03
        # These should be valid values
        self.assertLessEqual(range_val, 12)
        self.assertLessEqual(filter_val, 3)

    def test_silence_encoding(self):
        """Silence should encode to mostly zeros."""
        samples = [0] * 16
        block, _ = encode_brr_block(samples, (0, 0))
        # Data bytes (bytes 1-8) should be zeros for silence
        data_bytes = block[1:]
        self.assertTrue(all(b == 0 for b in data_bytes))

    def test_encode_brr_multiple_blocks(self):
        """Multiple blocks should encode correctly."""
        samples = [0] * 32  # 2 blocks worth
        brr_data = encode_brr(samples, loop=False)
        self.assertEqual(len(brr_data), 18)  # 2 * 9 bytes

    def test_encode_brr_end_flag(self):
        """Last block should have end flag set."""
        samples = [0] * 32
        brr_data = encode_brr(samples, loop=False)
        # Last block's header should have end flag (bit 0)
        last_header = brr_data[-9]
        self.assertEqual(last_header & 0x01, 0x01)

    def test_encode_brr_loop_flag(self):
        """Loop mode should set loop flag on last block."""
        samples = [0] * 32
        brr_data = encode_brr(samples, loop=True)
        # Last block's header should have both end and loop flags (bits 0-1)
        last_header = brr_data[-9]
        self.assertEqual(last_header & 0x03, 0x03)

    def test_encode_brr_padding(self):
        """Samples not multiple of 16 should be padded."""
        samples = [0] * 20  # Not a multiple of 16
        brr_data = encode_brr(samples, loop=False)
        # Should be padded to 32 samples = 2 blocks = 18 bytes
        self.assertEqual(len(brr_data), 18)

    def test_prev_samples_continuity(self):
        """Previous samples should be passed between blocks."""
        samples = [1000] * 32
        # This tests that the filter predictor works across blocks
        brr_data = encode_brr(samples, loop=False)
        # Just verify it doesn't crash and produces valid output
        self.assertEqual(len(brr_data), 18)


class TestAssemblyOutput(unittest.TestCase):
    """Test assembly file generation."""

    def test_basic_output(self):
        """Generate basic assembly output."""
        brr_data = bytes(9)  # One empty block

        output = format_assembly(brr_data, "test_sample", 16000, False, 0)

        self.assertIn(".segment \"RODATA\"", output)
        self.assertIn(".global test_sample_brr", output)
        self.assertIn("test_sample_sample_rate = 16000", output)

    def test_output_with_loop(self):
        """Generate output with loop information."""
        brr_data = bytes(18)  # Two blocks

        output = format_assembly(brr_data, "loop_sample", 32000, True, 16)

        self.assertIn("loop_sample_loop_start = 16", output)

    def test_size_calculation(self):
        """Size should be calculated correctly."""
        brr_data = bytes(27)  # Three blocks

        output = format_assembly(brr_data, "sized", 16000, False, 0)

        self.assertIn("sized_brr_size = sized_brr_end - sized_brr", output)


class TestWavReading(unittest.TestCase):
    """Test WAV file reading (requires creating temp files)."""

    def create_wav(self, samples, sample_rate=16000, channels=1, sample_width=2):
        """Create a temporary WAV file."""
        import wave
        import tempfile

        tf = tempfile.NamedTemporaryFile(suffix='.wav', delete=False)

        with wave.open(tf.name, 'wb') as wav:
            wav.setnchannels(channels)
            wav.setsampwidth(sample_width)
            wav.setframerate(sample_rate)

            if sample_width == 1:
                data = bytes([s + 128 for s in samples])  # Convert to unsigned
            elif sample_width == 2:
                data = b''.join(struct.pack('<h', s) for s in samples)
            else:
                raise ValueError("Unsupported sample width for test")

            wav.writeframes(data)

        return tf.name

    def test_read_16bit_wav(self):
        """Read 16-bit mono WAV."""
        samples = [0, 1000, -1000, 32767, -32768]
        filename = self.create_wav(samples, 16000, 1, 2)

        try:
            result, rate, channels = read_wav(filename)
            self.assertEqual(rate, 16000)
            self.assertEqual(channels, 1)
            self.assertEqual(result, samples)
        finally:
            Path(filename).unlink()

    def test_read_8bit_wav(self):
        """Read 8-bit WAV and convert to 16-bit."""
        # 8-bit unsigned: 128 = 0, 255 = max positive, 0 = max negative
        samples_8bit = [128, 255, 0, 192, 64]  # 0, +max, -max, +half, -half
        filename = self.create_wav([s - 128 for s in samples_8bit], 16000, 1, 1)

        try:
            result, rate, channels = read_wav(filename)
            self.assertEqual(rate, 16000)
            # Values should be scaled to 16-bit range
            self.assertEqual(result[0], 0)  # 128 -> 0
            self.assertEqual(result[1], 127 * 256)  # 255 -> +32512
            self.assertEqual(result[2], -128 * 256)  # 0 -> -32768
        finally:
            Path(filename).unlink()


if __name__ == '__main__':
    unittest.main()
