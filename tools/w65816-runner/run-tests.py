#!/usr/bin/env python3
"""
W65816 Integration Test Runner

Compiles LLVM IR test files to W65816 binaries and executes them
in the 816CE emulator, checking results against expected values.

Test file format:
    ; INTEGRATION-TEST
    ; EXPECT: <value>
    ; EXPECT-SIGNED: <value>  (optional, for signed interpretation)

    target triple = "w65816-unknown-none"

    define i16 @test_main() {
      ...
      ret i16 <result>
    }
"""

import argparse
import os
import re
import struct
import subprocess
import sys
import tempfile
from pathlib import Path
from concurrent.futures import ThreadPoolExecutor, as_completed

# ANSI colors
class Colors:
    GREEN = '\033[92m'
    RED = '\033[91m'
    YELLOW = '\033[93m'
    BLUE = '\033[94m'
    RESET = '\033[0m'
    BOLD = '\033[1m'

def colorize(text, color, bold=False):
    """Add ANSI color codes if stdout is a tty."""
    if not sys.stdout.isatty():
        return text
    prefix = Colors.BOLD if bold else ''
    return f"{prefix}{color}{text}{Colors.RESET}"

class TestResult:
    def __init__(self, name, passed, expected, actual, cycles=0, error=None):
        self.name = name
        self.passed = passed
        self.expected = expected
        self.actual = actual
        self.cycles = cycles
        self.error = error

def find_tools(build_dir):
    """Locate required tools."""
    tools = {}

    # LLVM tools in build directory
    for tool in ['llc', 'llvm-mc']:
        path = Path(build_dir) / 'bin' / tool
        if not path.exists():
            raise FileNotFoundError(f"{tool} not found at {path}")
        tools[tool] = str(path)

    return tools

def parse_test_file(path):
    """Extract test metadata from LLVM IR file."""
    with open(path, 'r') as f:
        content = f.read()

    metadata = {
        'is_test': False,
        'expect': None,
        'expect_signed': None,
        'skip': False,
        'skip_reason': None,
    }

    if '; INTEGRATION-TEST' in content:
        metadata['is_test'] = True

    # Look for expected value
    match = re.search(r'; EXPECT:\s*(-?\d+|0x[0-9a-fA-F]+)', content)
    if match:
        val = match.group(1)
        metadata['expect'] = int(val, 0)

    match = re.search(r'; EXPECT-SIGNED:\s*(-?\d+)', content)
    if match:
        metadata['expect_signed'] = int(match.group(1))

    # Check for skip directive
    match = re.search(r'; SKIP:\s*(.*)', content)
    if match:
        metadata['skip'] = True
        metadata['skip_reason'] = match.group(1).strip()

    return metadata

def parse_elf(elf_data):
    """Parse ELF file and return sections and symbols."""
    ELF_MAGIC = b'\x7fELF'
    if elf_data[:4] != ELF_MAGIC:
        raise ValueError("Not an ELF file")

    # ELF32 header offsets
    e_shoff = struct.unpack_from('<I', elf_data, 32)[0]
    e_shentsize = struct.unpack_from('<H', elf_data, 46)[0]
    e_shnum = struct.unpack_from('<H', elf_data, 48)[0]
    e_shstrndx = struct.unpack_from('<H', elf_data, 50)[0]

    # Read section headers
    sections = []
    for i in range(e_shnum):
        offset = e_shoff + i * e_shentsize
        sh_name = struct.unpack_from('<I', elf_data, offset)[0]
        sh_type = struct.unpack_from('<I', elf_data, offset + 4)[0]
        sh_flags = struct.unpack_from('<I', elf_data, offset + 8)[0]
        sh_addr = struct.unpack_from('<I', elf_data, offset + 12)[0]
        sh_offset = struct.unpack_from('<I', elf_data, offset + 16)[0]
        sh_size = struct.unpack_from('<I', elf_data, offset + 20)[0]
        sh_link = struct.unpack_from('<I', elf_data, offset + 24)[0]
        sh_info = struct.unpack_from('<I', elf_data, offset + 28)[0]
        sh_addralign = struct.unpack_from('<I', elf_data, offset + 32)[0]
        sh_entsize = struct.unpack_from('<I', elf_data, offset + 36)[0]
        sections.append({
            'index': i,
            'name_offset': sh_name,
            'type': sh_type,
            'flags': sh_flags,
            'addr': sh_addr,
            'offset': sh_offset,
            'size': sh_size,
            'link': sh_link,
            'info': sh_info,
            'addralign': sh_addralign,
            'entsize': sh_entsize,
        })

    # Get section string table and assign names
    if e_shstrndx < len(sections):
        strtab = sections[e_shstrndx]
        strtab_data = elf_data[strtab['offset']:strtab['offset'] + strtab['size']]
        for sec in sections:
            name_end = strtab_data.find(b'\x00', sec['name_offset'])
            if name_end == -1:
                name_end = len(strtab_data)
            sec['name'] = strtab_data[sec['name_offset']:name_end].decode('ascii', errors='ignore')

    return sections

def get_symbols(elf_data, sections):
    """Extract symbols from ELF file."""
    symbols = []

    # Find symbol table
    symtab = None
    strtab = None
    for sec in sections:
        if sec.get('name') == '.symtab':
            symtab = sec
        elif sec.get('name') == '.strtab':
            strtab = sec

    if not symtab or not strtab:
        return symbols

    strtab_data = elf_data[strtab['offset']:strtab['offset'] + strtab['size']]

    # Parse symbols (ELF32_Sym is 16 bytes)
    sym_offset = symtab['offset']
    num_symbols = symtab['size'] // 16
    for i in range(num_symbols):
        offset = sym_offset + i * 16
        st_name = struct.unpack_from('<I', elf_data, offset)[0]
        st_value = struct.unpack_from('<I', elf_data, offset + 4)[0]
        st_size = struct.unpack_from('<I', elf_data, offset + 8)[0]
        st_info = elf_data[offset + 12]
        st_other = elf_data[offset + 13]
        st_shndx = struct.unpack_from('<H', elf_data, offset + 14)[0]

        # Get symbol name
        name_end = strtab_data.find(b'\x00', st_name)
        if name_end == -1:
            name_end = len(strtab_data)
        name = strtab_data[st_name:name_end].decode('ascii', errors='ignore')

        symbols.append({
            'index': i,
            'name': name,
            'value': st_value,
            'size': st_size,
            'type': st_info & 0xF,
            'bind': st_info >> 4,
            'shndx': st_shndx,
        })

    return symbols

def get_relocations(elf_data, sections):
    """Extract relocations from ELF file."""
    relocations = []

    for sec in sections:
        # SHT_RELA = 4
        if sec.get('type') != 4:
            continue
        if not sec.get('name', '').startswith('.rela'):
            continue

        # Parse relocations (ELF32_Rela is 12 bytes)
        rela_offset = sec['offset']
        num_relas = sec['size'] // 12
        for i in range(num_relas):
            offset = rela_offset + i * 12
            r_offset = struct.unpack_from('<I', elf_data, offset)[0]
            r_info = struct.unpack_from('<I', elf_data, offset + 4)[0]
            r_addend = struct.unpack_from('<i', elf_data, offset + 8)[0]  # signed

            relocations.append({
                'offset': r_offset,
                'sym_idx': r_info >> 8,
                'type': r_info & 0xFF,
                'addend': r_addend,
                'section': sec.get('name', ''),
            })

    return relocations

def extract_text_section(elf_data):
    """Extract .text section from ELF object file."""
    sections = parse_elf(elf_data)

    # Find .text section
    for sec in sections:
        if sec.get('name') == '.text':
            return elf_data[sec['offset']:sec['offset'] + sec['size']]

    raise ValueError("No .text section found")

def extract_data_section(elf_data):
    """Extract .data section from ELF object file."""
    sections = parse_elf(elf_data)

    for sec in sections:
        if sec.get('name') == '.data':
            return elf_data[sec['offset']:sec['offset'] + sec['size']]

    return b''

def apply_relocations(code_bytes, data_bytes, elf_data, code_addr, data_addr):
    """Apply relocations to code and data sections.

    Args:
        code_bytes: Raw .text section bytes
        data_bytes: Raw .data section bytes
        elf_data: Full ELF file data
        code_addr: Address where code will be loaded
        data_addr: Address where data will be loaded

    Returns:
        Tuple of (relocated_code, relocated_data)
    """
    code = bytearray(code_bytes)
    data = bytearray(data_bytes) if data_bytes else bytearray()

    sections = parse_elf(elf_data)
    symbols = get_symbols(elf_data, sections)
    relocations = get_relocations(elf_data, sections)

    # Build section address map
    section_addrs = {}
    for sec in sections:
        if sec.get('name') == '.text':
            section_addrs[sec['index']] = code_addr
        elif sec.get('name') == '.data':
            section_addrs[sec['index']] = data_addr

    # W65816 relocation types
    R_W65816_16 = 1
    R_W65816_24 = 2
    R_W65816_8_PCREL = 3
    R_W65816_16_PCREL = 4
    R_W65816_8 = 5

    for reloc in relocations:
        # Only process .rela.text for now
        if reloc['section'] != '.rela.text':
            continue

        sym_idx = reloc['sym_idx']
        if sym_idx >= len(symbols):
            continue

        sym = symbols[sym_idx]
        offset = reloc['offset']
        addend = reloc['addend']
        rtype = reloc['type']

        # Calculate symbol address
        if sym['shndx'] == 0:  # SHN_UNDEF
            # Undefined symbol - skip (would need external linking)
            continue

        sym_section_addr = section_addrs.get(sym['shndx'], 0)
        sym_addr = sym_section_addr + sym['value'] + addend

        # Apply relocation
        if rtype == R_W65816_16:
            if offset + 2 <= len(code):
                code[offset] = sym_addr & 0xFF
                code[offset + 1] = (sym_addr >> 8) & 0xFF
        elif rtype == R_W65816_24:
            if offset + 3 <= len(code):
                code[offset] = sym_addr & 0xFF
                code[offset + 1] = (sym_addr >> 8) & 0xFF
                code[offset + 2] = (sym_addr >> 16) & 0xFF
        elif rtype == R_W65816_8:
            if offset + 1 <= len(code):
                code[offset] = sym_addr & 0xFF
        elif rtype == R_W65816_8_PCREL:
            # PC-relative: target - (fixup_location + 1)
            # fixup_location in final binary = code_addr + offset
            pc = code_addr + offset + 1
            rel = sym_addr - pc
            if offset + 1 <= len(code):
                code[offset] = rel & 0xFF
        elif rtype == R_W65816_16_PCREL:
            # 16-bit PC-relative
            pc = code_addr + offset + 2
            rel = sym_addr - pc
            if offset + 2 <= len(code):
                code[offset] = rel & 0xFF
                code[offset + 1] = (rel >> 8) & 0xFF

    return bytes(code), bytes(data)

def find_test_main_offset(elf_data):
    """Find the offset of test_main within the .text section."""
    sections = parse_elf(elf_data)
    symbols = get_symbols(elf_data, sections)

    # Find .text section index
    text_section_idx = None
    for sec in sections:
        if sec.get('name') == '.text':
            text_section_idx = sec['index']
            break

    # Find test_main symbol
    for sym in symbols:
        if sym['name'] == 'test_main' and sym['shndx'] == text_section_idx:
            return sym['value']

    # Fallback: assume test_main is at the start
    return 0

def create_test_binary(code_bytes, data_bytes=b'', load_addr=0x8000, elf_data=None):
    """Create a complete test binary with startup code and vectors.

    Memory layout:
      $0000-$00FF: Zero page (result stored at $0000)
      $0100-$01FF: Stack
      $8000+:      Code (startup + test code)
      $FFFA:       Vectors (NMI, RESET, IRQ)
    """
    # We need to build startup code that jumps to test_main.
    # Startup layout:
    #   0-0:    CLC
    #   1-1:    XCE
    #   2-3:    REP #$30
    #   4-6:    LDX #$01FF
    #   7-7:    TXS
    #   8-10:   JSR addr (3 bytes)
    #   11-12:  STA $00 (2 bytes)
    #   13:     STP
    #   14+:    test_main code starts here

    # Calculate where code section will be loaded
    startup_len = 14  # Total length of startup code including JSR + post-call
    code_addr = load_addr + startup_len

    # Calculate where data will be
    data_addr = code_addr + len(code_bytes)

    # Find test_main offset within .text section
    test_main_offset = 0
    if elf_data:
        test_main_offset = find_test_main_offset(elf_data)

    # Apply relocations if ELF data is provided
    if elf_data:
        code_bytes, data_bytes = apply_relocations(
            code_bytes, data_bytes, elf_data, code_addr, data_addr
        )

    test_main_addr = code_addr + test_main_offset

    # Build complete startup code
    startup_complete = bytes([
        0x18,               # CLC - clear carry for native mode switch
        0xFB,               # XCE - switch to native mode
        0xC2, 0x30,         # REP #$30 - 16-bit A and XY
        0xA2, 0xFF, 0x01,   # LDX #$01FF - initialize stack
        0x9A,               # TXS - transfer to stack pointer
        0x20,               # JSR opcode
        test_main_addr & 0xFF,
        (test_main_addr >> 8) & 0xFF,
        # After RTS from test_main, A contains result
        0x85, 0x00,         # STA $00 - store result to address 0
        0xDB,               # STP - stop CPU
    ])

    # Build the ROM image
    # We need to fill from load_addr to $FFFF
    rom_size = 0x10000 - load_addr

    rom = bytearray(rom_size)

    # Copy startup code at the beginning
    rom[0:len(startup_complete)] = startup_complete

    # Copy test code after startup
    code_offset = len(startup_complete)
    rom[code_offset:code_offset + len(code_bytes)] = code_bytes

    # Copy data after code (if any)
    if data_bytes:
        data_offset = code_offset + len(code_bytes)
        rom[data_offset:data_offset + len(data_bytes)] = data_bytes

    # Set up vectors at the end of ROM
    # Vectors are at $FFFA, $FFFC, $FFFE relative to memory
    # In our ROM they're at offsets 0x7FFA, 0x7FFC, 0x7FFE
    vectors_offset = 0x7FFA
    # NMI vector ($FFFA) - point to STP
    rom[vectors_offset:vectors_offset + 2] = bytes([load_addr & 0xFF, (load_addr >> 8) & 0xFF])
    # RESET vector ($FFFC) - point to start
    rom[vectors_offset + 2:vectors_offset + 4] = bytes([load_addr & 0xFF, (load_addr >> 8) & 0xFF])
    # IRQ vector ($FFFE) - point to STP
    rom[vectors_offset + 4:vectors_offset + 6] = bytes([load_addr & 0xFF, (load_addr >> 8) & 0xFF])

    return bytes(rom)

def run_test(test_file, tools, runner_dir, runner_bin, verbose=False):
    """Compile and run a single test."""
    test_name = Path(test_file).stem

    # Parse test metadata
    metadata = parse_test_file(test_file)

    if not metadata['is_test']:
        return TestResult(test_name, None, None, None, error="Not a test file")

    if metadata['skip']:
        return TestResult(test_name, None, None, None,
                         error=f"SKIPPED: {metadata['skip_reason']}")

    expected = metadata['expect']
    if expected is None:
        return TestResult(test_name, None, None, None, error="No EXPECT directive")

    with tempfile.TemporaryDirectory() as tmpdir:
        try:
            obj_file = os.path.join(tmpdir, f"{test_name}.o")
            bin_file = os.path.join(tmpdir, f"{test_name}.bin")

            # Compile LLVM IR directly to object file
            result = subprocess.run(
                [tools['llc'], '-march=w65816', '-filetype=obj', test_file, '-o', obj_file],
                capture_output=True, text=True, timeout=30
            )
            if result.returncode != 0:
                return TestResult(test_name, False, expected, None,
                                error=f"llc failed: {result.stderr}")

            # Read ELF object file
            with open(obj_file, 'rb') as f:
                elf_data = f.read()

            # Extract sections
            try:
                code_bytes = extract_text_section(elf_data)
            except ValueError as e:
                return TestResult(test_name, False, expected, None,
                                error=f"ELF extraction failed: {e}")

            data_bytes = extract_data_section(elf_data)

            # Create complete test binary with relocations applied
            binary = create_test_binary(code_bytes, data_bytes, elf_data=elf_data)

            with open(bin_file, 'wb') as f:
                f.write(binary)

            # Run in emulator
            cmd = [runner_bin, '--expect', str(expected), bin_file]
            if verbose:
                cmd.insert(1, '--verbose')

            result = subprocess.run(
                cmd, capture_output=True, text=True, timeout=60
            )

            # Parse result
            output = result.stdout + result.stderr

            if 'PASS' in output:
                match = re.search(r'\[(\d+) cycles\]', output)
                cycles = int(match.group(1)) if match else 0
                return TestResult(test_name, True, expected, expected, cycles)
            elif 'FAIL' in output:
                match = re.search(r'result=(-?\d+)', output)
                actual = int(match.group(1)) if match else None
                match = re.search(r'\[(\d+) cycles\]', output)
                cycles = int(match.group(1)) if match else 0
                return TestResult(test_name, False, expected, actual, cycles)
            elif 'TIMEOUT' in output:
                return TestResult(test_name, False, expected, None,
                                error="Execution timeout")
            else:
                return TestResult(test_name, False, expected, None,
                                error=f"Unknown result: {output[:200]}")

        except subprocess.TimeoutExpired:
            return TestResult(test_name, False, expected, None,
                            error="Process timeout")
        except Exception as e:
            return TestResult(test_name, False, expected, None,
                            error=str(e))

def main():
    parser = argparse.ArgumentParser(description='W65816 Integration Test Runner')
    parser.add_argument('tests', nargs='*', help='Test files to run (default: all)')
    parser.add_argument('-b', '--build-dir', default='build',
                       help='LLVM build directory (default: build)')
    parser.add_argument('-r', '--runner-dir', default='tools/w65816-runner',
                       help='Runner tool directory')
    parser.add_argument('-v', '--verbose', action='store_true',
                       help='Verbose output')
    parser.add_argument('-j', '--jobs', type=int, default=1,
                       help='Parallel jobs (default: 1)')
    parser.add_argument('--list', action='store_true',
                       help='List tests without running')
    args = parser.parse_args()

    # Find the runner binary
    runner_bin = os.path.join(args.build_dir, 'bin', 'w65816-runner')
    if not os.path.exists(runner_bin):
        print(f"Error: Runner not found at {runner_bin}")
        print("Build it with: make build-test-runner")
        return 1

    # Find tools
    try:
        tools = find_tools(args.build_dir)
    except FileNotFoundError as e:
        print(f"Error: {e}")
        return 1

    # Find test files
    if args.tests:
        test_files = args.tests
    else:
        test_dir = Path('test/integration/tests')
        if not test_dir.exists():
            print(f"Error: Test directory not found: {test_dir}")
            return 1
        test_files = sorted(test_dir.glob('**/*.ll'))

    if not test_files:
        print("No test files found")
        return 1

    if args.list:
        print(f"Found {len(test_files)} test files:")
        for tf in test_files:
            meta = parse_test_file(tf)
            status = "TEST" if meta['is_test'] else "skip"
            expect = meta['expect'] if meta['expect'] is not None else "?"
            print(f"  {tf}: {status} (expect: {expect})")
        return 0

    # Run tests
    print(f"Running {len(test_files)} integration tests...\n")

    results = []
    if args.jobs > 1:
        with ThreadPoolExecutor(max_workers=args.jobs) as executor:
            futures = {
                executor.submit(run_test, str(tf), tools, args.runner_dir,
                               runner_bin, args.verbose): tf
                for tf in test_files
            }
            for future in as_completed(futures):
                results.append(future.result())
    else:
        for tf in test_files:
            result = run_test(str(tf), tools, args.runner_dir, runner_bin,
                            args.verbose)
            results.append(result)

            # Print result immediately
            if result.passed is None:
                if result.error and result.error.startswith("SKIPPED"):
                    status = colorize("SKIP", Colors.YELLOW)
                else:
                    status = colorize("----", Colors.BLUE)
            elif result.passed:
                status = colorize("PASS", Colors.GREEN)
            else:
                status = colorize("FAIL", Colors.RED)

            name = result.name.ljust(30)
            if result.passed:
                print(f"  {status} {name} = {result.actual} [{result.cycles} cycles]")
            elif result.error:
                print(f"  {status} {name} {result.error}")
            else:
                print(f"  {status} {name} expected {result.expected}, got {result.actual}")

    # Summary
    passed = sum(1 for r in results if r.passed is True)
    failed = sum(1 for r in results if r.passed is False)
    skipped = sum(1 for r in results if r.passed is None)

    print()
    print("=" * 60)
    summary = f"Results: {passed} passed, {failed} failed, {skipped} skipped"
    if failed > 0:
        print(colorize(summary, Colors.RED, bold=True))
    else:
        print(colorize(summary, Colors.GREEN, bold=True))

    return 0 if failed == 0 else 1

if __name__ == '__main__':
    sys.exit(main())
