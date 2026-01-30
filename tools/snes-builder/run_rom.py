#!/usr/bin/env python3
"""
Launch SNES ROM in an emulator.
Supports Ares (default) and Snes9x.
"""

import subprocess
import sys
import os
from pathlib import Path

EMULATORS = {
    'ares': {
        'app': '/Applications/ares.app',
        'name': 'Ares',
    },
    'snes9x': {
        'app': '/Applications/Snes9x.app',
        'name': 'Snes9x',
    },
}

DEFAULT_EMULATOR = 'ares'

def find_emulator(preferred=None):
    """Find an available emulator, preferring the specified one."""
    order = [preferred] if preferred else []
    order += [e for e in [DEFAULT_EMULATOR] + list(EMULATORS.keys()) if e not in order]

    for emu_id in order:
        if emu_id in EMULATORS:
            app_path = EMULATORS[emu_id]['app']
            if os.path.exists(app_path):
                return emu_id, app_path

    return None, None

def run_rom(rom_path, emulator=None):
    """Launch ROM in the specified or default emulator."""
    rom_path = os.path.abspath(rom_path)

    if not os.path.exists(rom_path):
        print(f"Error: ROM not found: {rom_path}")
        return False

    emu_id, app_path = find_emulator(emulator)

    if not emu_id:
        print("Error: No supported emulator found.")
        print("Install Ares: brew install --cask ares-emulator")
        print("Install Snes9x: brew install --cask snes9x")
        return False

    emu_name = EMULATORS[emu_id]['name']
    print(f"Launching {rom_path}")
    print(f"Emulator: {emu_name} ({app_path})")

    # Use 'open' command to launch the app with the ROM
    cmd = ['open', '-a', app_path, rom_path]

    try:
        subprocess.run(cmd, check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error launching emulator: {e}")
        return False

def main():
    import argparse
    parser = argparse.ArgumentParser(description='Launch SNES ROM in emulator')
    parser.add_argument('rom', nargs='?', help='Path to ROM file (.sfc)')
    parser.add_argument('-e', '--emulator', choices=list(EMULATORS.keys()),
                        help=f'Emulator to use (default: {DEFAULT_EMULATOR})')
    parser.add_argument('--list', action='store_true', help='List available emulators')

    args = parser.parse_args()

    if args.list:
        print("Supported emulators:")
        for emu_id, info in EMULATORS.items():
            status = "installed" if os.path.exists(info['app']) else "not found"
            default = " (default)" if emu_id == DEFAULT_EMULATOR else ""
            print(f"  {emu_id}: {info['name']} - {status}{default}")
        return

    if not args.rom:
        parser.error("ROM path is required")

    success = run_rom(args.rom, args.emulator)
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()
