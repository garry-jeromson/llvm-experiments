#!/usr/bin/env python3
"""
W65816 C Integration Test Runner

Compiles C test files to W65816 binaries using Clang and executes them
in the 816CE emulator, checking results against expected values.

Test file format:
    // INTEGRATION-TEST
    // EXPECT: <value>
    // SKIP: <reason>  (optional)

    int test_main(void) {
        ...
        return <result>;
    }

Pipeline:
    1. clang -target w65816-unknown-none -O2 -c test.c -o test.o
    2. Parse ELF, apply relocations
    3. Build ROM image
    4. Execute in 816CE emulator
    5. Compare result with EXPECT value
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

# Runtime library configuration
# Placed at $C000 in ROM
RUNTIME_BASE_ADDR = 0xC000

# Global to hold loaded runtime binary and symbols
_runtime_binary = None
_runtime_symbols = None

def parse_runtime_map(map_path, listing_path):
    """Parse ld65 map file and ca65 listing to extract symbol addresses."""
    symbols = {}

    # Get CODE segment start address from map file
    code_start = RUNTIME_BASE_ADDR
    with open(map_path, 'r') as f:
        for line in f:
            # Look for CODE segment line: CODE  00C000  00C166  000167  00001
            if line.strip().startswith('CODE'):
                parts = line.split()
                if len(parts) >= 2:
                    try:
                        code_start = int(parts[1], 16)
                    except ValueError:
                        pass
                break

    # Parse listing file to get function offsets
    # Lines look like: 000000r 1               .proc __mulhi3
    with open(listing_path, 'r') as f:
        for line in f:
            if '.proc ' in line:
                parts = line.split()
                if len(parts) >= 3 and parts[2] == '.proc':
                    offset_str = parts[0].rstrip('r')  # Remove 'r' suffix
                    func_name = parts[3] if len(parts) > 3 else ''
                    try:
                        offset = int(offset_str, 16)
                        if func_name:
                            symbols[func_name] = code_start + offset
                    except ValueError:
                        continue

    return symbols

def load_runtime(build_dir):
    """Load the runtime library binary and symbol table from the build directory."""
    global _runtime_binary, _runtime_symbols
    if _runtime_binary is not None:
        return _runtime_binary, _runtime_symbols

    runtime_bin = Path(build_dir) / 'w65816-runtime' / 'w65816_runtime.bin'
    runtime_map = Path(build_dir) / 'w65816-runtime' / 'w65816_runtime.map'
    runtime_lst = Path(build_dir) / 'w65816-runtime' / 'w65816_runtime.lst'

    if not runtime_bin.exists():
        raise FileNotFoundError(
            f"Runtime binary not found at {runtime_bin}\n"
            "Build it with: make build-c-runtime"
        )

    if not runtime_map.exists():
        raise FileNotFoundError(
            f"Runtime map file not found at {runtime_map}\n"
            "Build it with: make build-c-runtime"
        )

    if not runtime_lst.exists():
        raise FileNotFoundError(
            f"Runtime listing file not found at {runtime_lst}\n"
            "Build it with: make build-c-runtime"
        )

    with open(runtime_bin, 'rb') as f:
        _runtime_binary = f.read()

    _runtime_symbols = parse_runtime_map(runtime_map, runtime_lst)

    return _runtime_binary, _runtime_symbols

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
    def __init__(self, name, passed, expected, actual, cycles=0, error=None, opt_level=None):
        self.name = name
        self.passed = passed
        self.expected = expected
        self.actual = actual
        self.cycles = cycles
        self.error = error
        self.opt_level = opt_level

def find_tools(build_dir):
    """Locate required tools."""
    tools = {}

    # LLVM tools in build directory
    for tool in ['clang', 'llc']:
        path = Path(build_dir) / 'bin' / tool
        if not path.exists():
            raise FileNotFoundError(f"{tool} not found at {path}")
        tools[tool] = str(path)

    return tools

def parse_test_file(path):
    """Extract test metadata from C file."""
    with open(path, 'r') as f:
        content = f.read()

    metadata = {
        'is_test': False,
        'expect': None,
        'skip': False,
        'skip_reason': None,
        'skip_at': set(),  # Opt levels to skip (e.g., {"O1", "O2"})
        'skip_at_reason': None,
        'expect_error': None,  # Expected compilation error pattern
    }

    if '// INTEGRATION-TEST' in content:
        metadata['is_test'] = True

    # Look for expected value
    match = re.search(r'// EXPECT:\s*(-?\d+|0x[0-9a-fA-F]+)', content)
    if match:
        val = match.group(1)
        metadata['expect'] = int(val, 0)

    # Look for expected compilation error (for error tests)
    match = re.search(r'// EXPECT-ERROR:\s*(.+)', content)
    if match:
        metadata['expect_error'] = match.group(1).strip()

    # Check for skip directive
    match = re.search(r'// SKIP:\s*(.*)', content)
    if match:
        metadata['skip'] = True
        metadata['skip_reason'] = match.group(1).strip()

    # Check for per-opt-level skip directive: // SKIP-AT: O1 O2 ...
    match = re.search(r'// SKIP-AT:\s*(.*)', content)
    if match:
        parts = match.group(1).strip()
        # Extract opt levels (e.g., "O1" or "O2 O3")
        # Allow a reason after a dash or parenthesized text
        levels = []
        reason_parts = []
        for part in parts.split():
            if part in ('O0', 'O1', 'O2', 'O3', 'Os', 'Oz'):
                levels.append(part)
            else:
                reason_parts.append(part)
        metadata['skip_at'] = set(levels)
        if reason_parts:
            metadata['skip_at_reason'] = ' '.join(reason_parts)

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

def extract_rodata_section(elf_data):
    """Extract .rodata section(s) from ELF object file.

    Handles both '.rodata' and variants like '.rodata.cst8'.
    """
    sections = parse_elf(elf_data)
    rodata_bytes = b''

    for sec in sections:
        name = sec.get('name', '')
        if name == '.rodata' or name.startswith('.rodata.'):
            rodata_bytes += elf_data[sec['offset']:sec['offset'] + sec['size']]

    return rodata_bytes

def get_bss_size(elf_data):
    """Get the size of the .bss section.

    BSS is NOBITS type, so it has no actual data in the file,
    just a size that needs to be allocated in memory.
    """
    sections = parse_elf(elf_data)

    for sec in sections:
        if sec.get('name') == '.bss':
            return sec['size']

    return 0

def apply_relocations(code_bytes, data_bytes, rodata_bytes, elf_data, code_addr, data_addr, rodata_addr, bss_addr=0, runtime_symbols=None):
    """Apply relocations to code, data, and rodata sections."""
    code = bytearray(code_bytes)
    data = bytearray(data_bytes) if data_bytes else bytearray()
    rodata = bytearray(rodata_bytes) if rodata_bytes else bytearray()

    if runtime_symbols is None:
        runtime_symbols = {}

    sections = parse_elf(elf_data)
    symbols = get_symbols(elf_data, sections)
    relocations = get_relocations(elf_data, sections)

    # Build section address map
    # Track rodata subsection offsets for proper address calculation
    section_addrs = {}
    rodata_offset = 0
    for sec in sections:
        name = sec.get('name', '')
        if name == '.text':
            section_addrs[sec['index']] = code_addr
        elif name == '.data':
            section_addrs[sec['index']] = data_addr
        elif name == '.rodata' or name.startswith('.rodata.'):
            # Handle .rodata and its variants (.rodata.cst8, etc.)
            section_addrs[sec['index']] = rodata_addr + rodata_offset
            rodata_offset += sec['size']
        elif name == '.bss':
            # BSS section - uninitialized data
            section_addrs[sec['index']] = bss_addr

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
            # Check if it's a runtime library symbol
            sym_name = sym['name']
            if sym_name in runtime_symbols:
                sym_addr = runtime_symbols[sym_name] + addend
            else:
                # Unknown undefined symbol - skip
                continue
        else:
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
            pc = code_addr + offset + 1
            rel = sym_addr - pc
            if offset + 1 <= len(code):
                code[offset] = rel & 0xFF
        elif rtype == R_W65816_16_PCREL:
            pc = code_addr + offset + 2
            rel = sym_addr - pc
            if offset + 2 <= len(code):
                code[offset] = rel & 0xFF
                code[offset + 1] = (rel >> 8) & 0xFF

    # Fix up same-section references (JSR/JSL to functions in .text)
    code_size = len(code)
    i = 0
    while i < len(code):
        opcode = code[i]
        if opcode == 0x20:  # JSR abs (3 bytes)
            if i + 2 < len(code):
                target = code[i + 1] | (code[i + 2] << 8)
                if target < code_size:
                    new_target = code_addr + target
                    code[i + 1] = new_target & 0xFF
                    code[i + 2] = (new_target >> 8) & 0xFF
            i += 3
        elif opcode == 0x22:  # JSL long (4 bytes)
            if i + 3 < len(code):
                target = code[i + 1] | (code[i + 2] << 8) | (code[i + 3] << 16)
                if target < code_size:
                    new_target = code_addr + target
                    code[i + 1] = new_target & 0xFF
                    code[i + 2] = (new_target >> 8) & 0xFF
                    code[i + 3] = (new_target >> 16) & 0xFF
            i += 4
        else:
            i += 1

    return bytes(code), bytes(data), bytes(rodata)

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

def create_test_binary(code_bytes, data_bytes=b'', rodata_bytes=b'', load_addr=0x8000, elf_data=None, runtime_binary=None, runtime_symbols=None):
    """Create a complete test binary with startup code and vectors."""
    # Startup layout:
    #   0:      CLC
    #   1:      XCE
    #   2-3:    REP #$30
    #   4-6:    LDX #$01FF
    #   7:      TXS
    #   8-10:   JSR addr (3 bytes)
    #   11-12:  STA $00 (2 bytes)
    #   13:     STP
    #   14+:    test_main code starts here

    startup_len = 14
    code_addr = load_addr + startup_len
    data_addr = code_addr + len(code_bytes)
    rodata_addr = data_addr + len(data_bytes)

    # BSS goes after rodata - it's uninitialized data that needs memory allocation
    bss_size = get_bss_size(elf_data) if elf_data else 0
    bss_addr = rodata_addr + len(rodata_bytes)

    test_main_offset = 0
    if elf_data:
        test_main_offset = find_test_main_offset(elf_data)

    if elf_data:
        code_bytes, data_bytes, rodata_bytes = apply_relocations(
            code_bytes, data_bytes, rodata_bytes, elf_data, code_addr, data_addr, rodata_addr,
            bss_addr=bss_addr, runtime_symbols=runtime_symbols
        )

    test_main_addr = code_addr + test_main_offset

    startup_complete = bytes([
        0x18,               # CLC
        0xFB,               # XCE
        0xC2, 0x30,         # REP #$30
        0xA2, 0xFF, 0x01,   # LDX #$01FF
        0x9A,               # TXS
        0x20,               # JSR opcode
        test_main_addr & 0xFF,
        (test_main_addr >> 8) & 0xFF,
        0x85, 0x00,         # STA $00
        0xDB,               # STP
    ])

    rom_size = 0x10000 - load_addr
    rom = bytearray(rom_size)

    # Copy startup code
    rom[0:len(startup_complete)] = startup_complete

    # Copy test code after startup
    code_offset = len(startup_complete)
    rom[code_offset:code_offset + len(code_bytes)] = code_bytes

    # Copy data after code
    if data_bytes:
        data_offset = code_offset + len(code_bytes)
        rom[data_offset:data_offset + len(data_bytes)] = data_bytes

    # Copy rodata after data
    if rodata_bytes:
        rodata_offset = code_offset + len(code_bytes) + len(data_bytes)
        rom[rodata_offset:rodata_offset + len(rodata_bytes)] = rodata_bytes

    # Copy runtime library at $C000 (offset $4000 from $8000)
    if runtime_binary:
        runtime_offset = RUNTIME_BASE_ADDR - load_addr
        rom[runtime_offset:runtime_offset + len(runtime_binary)] = runtime_binary

    # Set up vectors
    vectors_offset = 0x7FFA
    rom[vectors_offset:vectors_offset + 2] = bytes([load_addr & 0xFF, (load_addr >> 8) & 0xFF])
    rom[vectors_offset + 2:vectors_offset + 4] = bytes([load_addr & 0xFF, (load_addr >> 8) & 0xFF])
    rom[vectors_offset + 4:vectors_offset + 6] = bytes([load_addr & 0xFF, (load_addr >> 8) & 0xFF])

    return bytes(rom)

def run_error_test(test_file, test_name, expect_error, tools, opt_level='O2', extra_clang_flags=None):
    """Run a test that expects compilation to fail with a specific error message."""
    with tempfile.TemporaryDirectory() as tmpdir:
        try:
            obj_file = os.path.join(tmpdir, f"{test_name}.o")

            # Try to compile C to object file
            clang_cmd = [tools['clang'], '-target', 'w65816-unknown-none', f'-{opt_level}',
                 '-c', test_file, '-o', obj_file]
            if extra_clang_flags:
                clang_cmd[1:1] = extra_clang_flags
            result = subprocess.run(
                clang_cmd,
                capture_output=True, text=True, timeout=30
            )

            # Combine stdout and stderr for error checking
            output = result.stdout + result.stderr

            if result.returncode == 0:
                # Compilation succeeded when it should have failed
                return TestResult(test_name, False, f"error: {expect_error}", "compilation succeeded",
                                error="Expected compilation to fail", opt_level=opt_level)

            # Compilation failed - check if the error message matches
            if expect_error in output:
                return TestResult(test_name, True, f"error: {expect_error}",
                                f"error: {expect_error}", cycles=0, opt_level=opt_level)
            else:
                # Wrong error message
                return TestResult(test_name, False, f"error: {expect_error}", output[:200],
                                error=f"Wrong error message", opt_level=opt_level)

        except subprocess.TimeoutExpired:
            return TestResult(test_name, False, f"error: {expect_error}", None,
                            error="Process timeout", opt_level=opt_level)
        except Exception as e:
            return TestResult(test_name, False, f"error: {expect_error}", None,
                            error=str(e), opt_level=opt_level)

def run_test(test_file, tools, runner_bin, build_dir, verbose=False, opt_level='O2', extra_clang_flags=None):
    """Compile and run a single test at specified optimization level."""
    test_name = Path(test_file).stem

    metadata = parse_test_file(test_file)

    if not metadata['is_test']:
        return TestResult(test_name, None, None, None, error="Not a test file", opt_level=opt_level)

    if metadata['skip']:
        return TestResult(test_name, None, None, None,
                         error=f"SKIPPED: {metadata['skip_reason']}", opt_level=opt_level)

    if opt_level in metadata.get('skip_at', set()):
        reason = metadata.get('skip_at_reason', f'SKIP-AT {opt_level}')
        return TestResult(test_name, None, None, None,
                         error=f"SKIPPED: {reason} (at -{opt_level})", opt_level=opt_level)

    # Handle error tests (tests that expect compilation to fail)
    expect_error = metadata.get('expect_error')
    if expect_error:
        return run_error_test(test_file, test_name, expect_error, tools, opt_level, extra_clang_flags)

    expected = metadata['expect']
    if expected is None:
        return TestResult(test_name, None, None, None, error="No EXPECT directive", opt_level=opt_level)

    # Load runtime library and symbols
    try:
        runtime_binary, runtime_symbols = load_runtime(build_dir)
    except FileNotFoundError as e:
        return TestResult(test_name, False, expected, None, error=str(e), opt_level=opt_level)

    with tempfile.TemporaryDirectory() as tmpdir:
        try:
            obj_file = os.path.join(tmpdir, f"{test_name}.o")
            bin_file = os.path.join(tmpdir, f"{test_name}.bin")

            # Compile C to object file
            clang_cmd = [tools['clang'], '-target', 'w65816-unknown-none', f'-{opt_level}',
                 '-c', test_file, '-o', obj_file]
            if extra_clang_flags:
                clang_cmd[1:1] = extra_clang_flags
            result = subprocess.run(
                clang_cmd,
                capture_output=True, text=True, timeout=30
            )
            if result.returncode != 0:
                return TestResult(test_name, False, expected, None,
                                error=f"clang failed: {result.stderr}", opt_level=opt_level)

            # Read ELF object file
            with open(obj_file, 'rb') as f:
                elf_data = f.read()

            # Extract sections
            try:
                code_bytes = extract_text_section(elf_data)
            except ValueError as e:
                return TestResult(test_name, False, expected, None,
                                error=f"ELF extraction failed: {e}", opt_level=opt_level)

            data_bytes = extract_data_section(elf_data)
            rodata_bytes = extract_rodata_section(elf_data)

            # Create complete test binary with runtime library
            binary = create_test_binary(code_bytes, data_bytes, rodata_bytes,
                                       elf_data=elf_data, runtime_binary=runtime_binary,
                                       runtime_symbols=runtime_symbols)

            with open(bin_file, 'wb') as f:
                f.write(binary)

            # Run in emulator
            cmd = [runner_bin, '--expect', str(expected), bin_file]
            if verbose:
                cmd.insert(1, '--verbose')

            result = subprocess.run(
                cmd, capture_output=True, text=True, timeout=60
            )

            output = result.stdout + result.stderr

            if 'PASS' in output:
                match = re.search(r'\[(\d+) cycles\]', output)
                cycles = int(match.group(1)) if match else 0
                return TestResult(test_name, True, expected, expected, cycles, opt_level=opt_level)
            elif 'FAIL' in output:
                match = re.search(r'result=(-?\d+)', output)
                actual = int(match.group(1)) if match else None
                match = re.search(r'\[(\d+) cycles\]', output)
                cycles = int(match.group(1)) if match else 0
                return TestResult(test_name, False, expected, actual, cycles, opt_level=opt_level)
            elif 'TIMEOUT' in output:
                return TestResult(test_name, False, expected, None,
                                error="Execution timeout", opt_level=opt_level)
            else:
                return TestResult(test_name, False, expected, None,
                                error=f"Unknown result: {output[:200]}", opt_level=opt_level)

        except subprocess.TimeoutExpired:
            return TestResult(test_name, False, expected, None,
                            error="Process timeout", opt_level=opt_level)
        except Exception as e:
            return TestResult(test_name, False, expected, None,
                            error=str(e), opt_level=opt_level)

def _print_result(result, show_opt_level=False):
    """Print a single test result."""
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

def main():
    parser = argparse.ArgumentParser(description='W65816 C Integration Test Runner')
    parser.add_argument('tests', nargs='*', help='Test files to run (default: all)')
    parser.add_argument('-b', '--build-dir', default='build',
                       help='LLVM build directory (default: build)')
    parser.add_argument('-v', '--verbose', action='store_true',
                       help='Verbose output')
    parser.add_argument('-j', '--jobs', type=int, default=1,
                       help='Parallel jobs (default: 1)')
    parser.add_argument('--list', action='store_true',
                       help='List tests without running')
    parser.add_argument('-O', '--opt-levels', nargs='+', default=['O2'],
                       choices=['O0', 'O1', 'O2', 'O3', 'Os', 'Oz'],
                       help='Optimization levels to test (default: O2)')
    parser.add_argument('--all-opts', action='store_true',
                       help='Test all supported optimization levels (O0, O1, O2, O3)')
    parser.add_argument('--clang-flags', type=str, default='',
                       help='Extra flags to pass to clang (e.g., "--clang-flags=-mllvm -global-isel")')
    args = parser.parse_args()

    # Handle --all-opts flag
    if args.all_opts:
        args.opt_levels = ['O0', 'O1', 'O2', 'O3']

    # Parse extra clang flags
    extra_clang_flags = args.clang_flags.split() if args.clang_flags else None

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
        test_dir = Path('test/c-integration/tests')
        if not test_dir.exists():
            print(f"Error: Test directory not found: {test_dir}")
            return 1
        test_files = sorted(test_dir.glob('**/*.c'))

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

    # Run tests at each optimization level
    opt_levels = args.opt_levels
    total_tests = len(test_files) * len(opt_levels)
    print(f"Running {len(test_files)} C integration tests at {len(opt_levels)} optimization level(s): {', '.join(opt_levels)}")
    print(f"Total test runs: {total_tests}\n")

    all_results = []  # (opt_level, results_list)

    for opt_level in opt_levels:
        if len(opt_levels) > 1:
            print(f"\n{'='*60}")
            print(f"  Optimization level: -{opt_level}")
            print(f"{'='*60}\n")

        results = []
        if args.jobs > 1:
            with ThreadPoolExecutor(max_workers=args.jobs) as executor:
                futures = {
                    executor.submit(run_test, str(tf), tools, runner_bin, args.build_dir, args.verbose, opt_level, extra_clang_flags): tf
                    for tf in test_files
                }
                for future in as_completed(futures):
                    results.append(future.result())
            # Sort by name for consistent output
            results.sort(key=lambda r: r.name)
            for result in results:
                _print_result(result, len(opt_levels) > 1)
        else:
            for tf in test_files:
                result = run_test(str(tf), tools, runner_bin, args.build_dir, args.verbose, opt_level, extra_clang_flags)
                results.append(result)
                _print_result(result, len(opt_levels) > 1)

        all_results.append((opt_level, results))

        # Print per-level summary
        passed = sum(1 for r in results if r.passed is True)
        failed = sum(1 for r in results if r.passed is False)
        skipped = sum(1 for r in results if r.passed is None)
        print()
        summary = f"-{opt_level}: {passed} passed, {failed} failed, {skipped} skipped"
        if failed > 0:
            print(colorize(summary, Colors.RED))
        else:
            print(colorize(summary, Colors.GREEN))

    # Overall summary
    print()
    print("=" * 60)
    print("SUMMARY BY OPTIMIZATION LEVEL")
    print("=" * 60)

    total_passed = 0
    total_failed = 0
    total_skipped = 0
    failures_by_level = {}

    for opt_level, results in all_results:
        passed = sum(1 for r in results if r.passed is True)
        failed = sum(1 for r in results if r.passed is False)
        skipped = sum(1 for r in results if r.passed is None)
        total_passed += passed
        total_failed += failed
        total_skipped += skipped

        # Track failures
        failures = [r for r in results if r.passed is False]
        if failures:
            failures_by_level[opt_level] = failures

        status = colorize("PASS", Colors.GREEN) if failed == 0 else colorize("FAIL", Colors.RED)
        print(f"  {status} -{opt_level}: {passed}/{len(results)} passed")

    # Show failures grouped by opt level
    if failures_by_level:
        print()
        print("FAILURES:")
        for opt_level, failures in failures_by_level.items():
            print(f"\n  -{opt_level}:")
            for r in failures:
                if r.error:
                    print(f"    {r.name}: {r.error[:60]}")
                else:
                    print(f"    {r.name}: expected {r.expected}, got {r.actual}")

    print()
    print("=" * 60)
    grand_total = f"Total: {total_passed} passed, {total_failed} failed, {total_skipped} skipped"
    if total_failed > 0:
        print(colorize(grand_total, Colors.RED, bold=True))
    else:
        print(colorize(grand_total, Colors.GREEN, bold=True))

    return 0 if failed == 0 else 1

if __name__ == '__main__':
    sys.exit(main())
