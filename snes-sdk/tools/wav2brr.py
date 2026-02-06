#!/usr/bin/env python3
"""
wav2brr.py - Convert WAV audio files to SNES BRR format

BRR (Bit Rate Reduction) is the audio compression format used by the SNES SPC700.
Each BRR block is 9 bytes: 1 header byte + 8 bytes of compressed samples (16 samples).

Usage:
    python wav2brr.py input.wav -o output.s [options]

Options:
    -o, --output      Output file (default: stdout)
    --sample-rate     Target sample rate in Hz (default: 16000)
    --loop            Make sample loop (adds loop point)
    --loop-start      Loop start sample (default: 0)
    --label           Label prefix for generated data
    --raw             Output raw BRR binary instead of assembly
"""

import argparse
import struct
import sys
from pathlib import Path
from typing import List, Tuple, Optional

try:
    import wave
except ImportError:
    print("Error: wave module not available", file=sys.stderr)
    sys.exit(1)


def read_wav(filename: str) -> Tuple[List[int], int, int]:
    """
    Read WAV file and return (samples, sample_rate, channels).
    Samples are normalized to signed 16-bit range.
    """
    with wave.open(filename, 'rb') as wav:
        channels = wav.getnchannels()
        sample_width = wav.getsampwidth()
        sample_rate = wav.getframerate()
        n_frames = wav.getnframes()

        raw_data = wav.readframes(n_frames)

    samples = []

    if sample_width == 1:
        # 8-bit unsigned
        for i in range(0, len(raw_data), channels):
            sample = raw_data[i]
            # Convert to signed 16-bit
            sample = (sample - 128) * 256
            samples.append(sample)
    elif sample_width == 2:
        # 16-bit signed
        for i in range(0, len(raw_data), 2 * channels):
            sample = struct.unpack('<h', raw_data[i:i+2])[0]
            samples.append(sample)
    elif sample_width == 3:
        # 24-bit signed
        for i in range(0, len(raw_data), 3 * channels):
            b = raw_data[i:i+3]
            sample = struct.unpack('<i', b + (b'\xff' if b[2] & 0x80 else b'\x00'))[0]
            sample = sample >> 8  # Convert to 16-bit
            samples.append(sample)
    elif sample_width == 4:
        # 32-bit signed
        for i in range(0, len(raw_data), 4 * channels):
            sample = struct.unpack('<i', raw_data[i:i+4])[0]
            sample = sample >> 16  # Convert to 16-bit
            samples.append(sample)
    else:
        raise ValueError(f"Unsupported sample width: {sample_width}")

    return samples, sample_rate, channels


def resample(samples: List[int], src_rate: int, dst_rate: int) -> List[int]:
    """Simple linear resampling."""
    if src_rate == dst_rate:
        return samples

    ratio = src_rate / dst_rate
    new_length = int(len(samples) / ratio)

    result = []
    for i in range(new_length):
        src_pos = i * ratio
        idx = int(src_pos)
        frac = src_pos - idx

        if idx + 1 < len(samples):
            sample = int(samples[idx] * (1 - frac) + samples[idx + 1] * frac)
        else:
            sample = samples[idx]

        result.append(sample)

    return result


def encode_brr_block(samples: List[int], prev_samples: Tuple[int, int]) -> Tuple[bytes, Tuple[int, int]]:
    """
    Encode 16 samples into a BRR block (9 bytes).

    BRR format:
    - Header byte: RRRR_FFEE
        R: Range/shift (0-12)
        F: Filter (0-3)
        E: End flag (1 = last block)
        E: Loop flag (1 = loop point)
    - 8 data bytes: 16 4-bit nibbles

    Returns (block_data, new_prev_samples)
    """
    if len(samples) != 16:
        raise ValueError(f"Expected 16 samples, got {len(samples)}")

    # Try different range/filter combinations and pick the best
    best_error = float('inf')
    best_block = None
    best_prev = prev_samples

    for range_val in range(13):
        for filter_val in range(4):
            error, block, new_prev = try_encode(samples, prev_samples, range_val, filter_val)
            if error < best_error:
                best_error = error
                best_block = block
                best_prev = new_prev

    return best_block, best_prev


def try_encode(samples: List[int], prev_samples: Tuple[int, int],
               range_val: int, filter_val: int) -> Tuple[float, bytes, Tuple[int, int]]:
    """Try encoding with specific range and filter, return error and encoded data."""
    shift = 12 - range_val
    p1, p2 = prev_samples

    nibbles = []
    decoded = []
    total_error = 0

    for sample in samples:
        # Apply inverse filter to get target encoded value
        if filter_val == 0:
            pred = 0
        elif filter_val == 1:
            pred = p1 + (-p1 >> 4)
        elif filter_val == 2:
            pred = (p1 << 1) + ((-((p1 << 1) + p1)) >> 5) - p2 + (p2 >> 4)
        elif filter_val == 3:
            pred = (p1 << 1) + ((-(p1 + (p1 << 2) + (p1 << 3))) >> 6) - p2 + (((p2 << 1) + p2) >> 4)
        else:
            pred = 0

        # Calculate needed encoded value
        target = sample - pred

        # Quantize to 4-bit signed
        if shift > 0:
            enc = target >> shift
        else:
            enc = target << (-shift)

        # Clamp to 4-bit signed range
        enc = max(-8, min(7, enc))
        nibbles.append(enc & 0x0F)

        # Decode to get actual output
        if shift > 0:
            dec = enc << shift
        else:
            dec = enc >> (-shift)

        dec = (dec << 1) >> 1  # Sign extend

        output = pred + dec

        # Clamp output
        output = max(-32768, min(32767, output))

        decoded.append(output)
        total_error += (output - sample) ** 2

        p2 = p1
        p1 = output

    # Pack nibbles into bytes
    header = (range_val << 4) | (filter_val << 2)
    data = bytes([header])

    for i in range(0, 16, 2):
        byte = ((nibbles[i] & 0x0F) << 4) | (nibbles[i + 1] & 0x0F)
        data += bytes([byte])

    return total_error, data, (p1, p2)


def encode_brr(samples: List[int], loop: bool = False, loop_start: int = 0) -> bytes:
    """
    Encode audio samples to BRR format.

    Returns bytes of BRR data.
    """
    # Pad samples to multiple of 16
    while len(samples) % 16 != 0:
        samples.append(0)

    blocks = []
    prev = (0, 0)

    for i in range(0, len(samples), 16):
        block_samples = samples[i:i + 16]
        block, prev = encode_brr_block(block_samples, prev)
        blocks.append(bytearray(block))

    # Set flags on last block
    if blocks:
        if loop:
            blocks[-1][0] |= 0x03  # End + Loop
        else:
            blocks[-1][0] |= 0x01  # End only

    # Set loop point
    if loop and loop_start > 0:
        loop_block = loop_start // 16
        if loop_block < len(blocks):
            blocks[loop_block][0] |= 0x02  # Loop flag

    return b''.join(blocks)


def format_assembly(brr_data: bytes, label: str, sample_rate: int,
                    loop: bool, loop_start: int) -> str:
    """Format BRR data as ca65 assembly."""
    lines = [
        f"; Generated by wav2brr.py",
        f"; Sample rate: {sample_rate} Hz",
        f"; Size: {len(brr_data)} bytes ({len(brr_data) // 9} blocks)",
        f"; Loop: {'yes' if loop else 'no'}",
        "",
        ".segment \"RODATA\"",
        "",
        f".global {label}_brr",
        f"{label}_brr:",
    ]

    # Output BRR data
    for i in range(0, len(brr_data), 9):
        block = brr_data[i:i+9]
        hex_str = ", ".join(f"${b:02X}" for b in block)
        lines.append(f"    .byte {hex_str}")

    lines.extend([
        f"{label}_brr_end:",
        f".global {label}_brr_size",
        f"{label}_brr_size = {label}_brr_end - {label}_brr",
        "",
        f".global {label}_sample_rate",
        f"{label}_sample_rate = {sample_rate}",
        "",
    ])

    if loop:
        lines.extend([
            f".global {label}_loop_start",
            f"{label}_loop_start = {loop_start}",
            "",
        ])

    return "\n".join(lines)


def main():
    parser = argparse.ArgumentParser(description="Convert WAV to SNES BRR format")
    parser.add_argument("input", help="Input WAV file")
    parser.add_argument("-o", "--output", help="Output file (default: stdout)")
    parser.add_argument("--sample-rate", type=int, default=16000,
                        help="Target sample rate (default: 16000)")
    parser.add_argument("--loop", action="store_true",
                        help="Make sample loop")
    parser.add_argument("--loop-start", type=int, default=0,
                        help="Loop start sample (default: 0)")
    parser.add_argument("--label", default="sample",
                        help="Label prefix (default: sample)")
    parser.add_argument("--raw", action="store_true",
                        help="Output raw BRR binary")

    args = parser.parse_args()

    # Read WAV
    try:
        samples, src_rate, channels = read_wav(args.input)
    except Exception as e:
        print(f"Error reading WAV: {e}", file=sys.stderr)
        sys.exit(1)

    print(f"Input: {len(samples)} samples at {src_rate} Hz", file=sys.stderr)

    # Resample if needed
    if src_rate != args.sample_rate:
        samples = resample(samples, src_rate, args.sample_rate)
        print(f"Resampled to {len(samples)} samples at {args.sample_rate} Hz", file=sys.stderr)

    # Encode to BRR
    brr_data = encode_brr(samples, args.loop, args.loop_start)
    print(f"Output: {len(brr_data)} bytes ({len(brr_data) // 9} BRR blocks)", file=sys.stderr)

    if args.raw:
        # Output raw binary
        if args.output:
            with open(args.output, 'wb') as f:
                f.write(brr_data)
        else:
            sys.stdout.buffer.write(brr_data)
    else:
        # Output assembly
        output = format_assembly(brr_data, args.label, args.sample_rate,
                                 args.loop, args.loop_start)

        if args.output:
            with open(args.output, 'w') as f:
                f.write(output)
        else:
            print(output)


if __name__ == "__main__":
    main()
