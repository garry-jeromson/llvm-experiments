#!/usr/bin/env python3
"""
Simple ELF to raw binary converter for W65816 integration tests.

Extracts .text section from ELF object file and outputs raw machine code.
"""

import struct
import sys

ELF_MAGIC = b'\x7fELF'

def read_elf_sections(data):
    """Parse ELF headers and return section name -> (offset, size) mapping."""
    if data[:4] != ELF_MAGIC:
        raise ValueError("Not an ELF file")

    # ELF32 header
    ei_class = data[4]
    if ei_class != 1:  # ELFCLASS32
        raise ValueError("Not a 32-bit ELF")

    # Get header values
    e_shoff = struct.unpack_from('<I', data, 32)[0]  # Section header offset
    e_shentsize = struct.unpack_from('<H', data, 46)[0]  # Section header entry size
    e_shnum = struct.unpack_from('<H', data, 48)[0]  # Number of section headers
    e_shstrndx = struct.unpack_from('<H', data, 50)[0]  # Section name string table index

    # Read section headers
    sections = []
    for i in range(e_shnum):
        offset = e_shoff + i * e_shentsize
        sh_name = struct.unpack_from('<I', data, offset)[0]
        sh_type = struct.unpack_from('<I', data, offset + 4)[0]
        sh_offset = struct.unpack_from('<I', data, offset + 16)[0]
        sh_size = struct.unpack_from('<I', data, offset + 20)[0]
        sections.append({
            'name_offset': sh_name,
            'type': sh_type,
            'offset': sh_offset,
            'size': sh_size
        })

    # Get string table
    if e_shstrndx < len(sections):
        strtab = sections[e_shstrndx]
        strtab_data = data[strtab['offset']:strtab['offset'] + strtab['size']]

        # Resolve section names
        for sec in sections:
            name_end = strtab_data.find(b'\x00', sec['name_offset'])
            sec['name'] = strtab_data[sec['name_offset']:name_end].decode('ascii')
    else:
        for sec in sections:
            sec['name'] = ''

    return sections

def main():
    if len(sys.argv) < 2:
        print(f"Usage: {sys.argv[0]} <elf-file> [output-file]", file=sys.stderr)
        sys.exit(1)

    elf_file = sys.argv[1]
    output_file = sys.argv[2] if len(sys.argv) > 2 else None

    with open(elf_file, 'rb') as f:
        data = f.read()

    sections = read_elf_sections(data)

    # Find .text section
    text_section = None
    for sec in sections:
        if sec['name'] == '.text':
            text_section = sec
            break

    if not text_section:
        print("Error: No .text section found", file=sys.stderr)
        sys.exit(1)

    # Extract raw code
    code = data[text_section['offset']:text_section['offset'] + text_section['size']]

    if output_file:
        with open(output_file, 'wb') as f:
            f.write(code)
    else:
        sys.stdout.buffer.write(code)

if __name__ == '__main__':
    main()
