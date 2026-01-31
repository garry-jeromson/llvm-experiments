#!/usr/bin/env python3
"""
Launch SNES ROM in Ares emulator.
"""

import subprocess
import sys
import os

ARES_APP = '/Applications/ares.app'

def run_rom(rom_path):
    """Launch ROM in Ares emulator."""
    rom_path = os.path.abspath(rom_path)

    if not os.path.exists(rom_path):
        print(f"Error: ROM not found: {rom_path}")
        return False

    if not os.path.exists(ARES_APP):
        print("Error: Ares emulator not found.")
        print("Install Ares: brew install --cask ares-emulator")
        return False

    print(f"Launching {rom_path}")
    print(f"Emulator: Ares ({ARES_APP})")

    cmd = ['open', '-a', ARES_APP, rom_path]

    try:
        subprocess.run(cmd, check=True)
        return True
    except subprocess.CalledProcessError as e:
        print(f"Error launching emulator: {e}")
        return False

def main():
    import argparse
    parser = argparse.ArgumentParser(description='Launch SNES ROM in Ares emulator')
    parser.add_argument('rom', help='Path to ROM file (.sfc)')

    args = parser.parse_args()

    success = run_rom(args.rom)
    sys.exit(0 if success else 1)

if __name__ == '__main__':
    main()
