# Unused Files

These files were created for a ca65/ld65-based linking approach but are currently
unused because:

1. LLVM's assembly output uses ELF-style directives (`.text`, `.type`, etc.)
   which ca65 doesn't understand
2. The backend doesn't emit relocations for local symbols, so linking
   multiple object files doesn't work correctly

The current approach uses llc's object file output directly and extracts
the raw .text section without needing a linker.

These files are kept in case we later implement:
- Proper relocation emission in the W65816 backend
- A compatibility layer between LLVM assembly and ca65

## Files

- `test_wrapper.s` - Assembly wrapper for test harness (sets up CPU, calls test_main)
- `test.cfg` - ld65 linker configuration for creating test binaries
