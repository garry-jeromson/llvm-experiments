# SNES SDK TODO

## Completed
- [x] Port stdlib functions (strlen, strcpy, strcmp, strncpy, strncmp, strcat, strchr)
- [x] String display driver (text.hpp, font_2bpp.s)

## Remaining
- [ ] Sound driver - Requires SPC700 assembly and APU communication protocol
- [ ] SuperFX driver - Requires SuperFX assembly and coprocessor interface
- [ ] Review SDK code for quality: readability, clear variable names, functions with purpose, no magic numbers, etc.

## Notes

### Sound Driver
The SNES uses a separate Sony SPC700 processor for audio. A sound driver requires:
1. SPC700 assembly code (uploaded via APUIO0-3 ports at $2140-$2143)
2. Boot/IPL code to initialize the APU
3. Music data format (SPC, IT, XM, or custom)
4. Communication protocol between 65816 and SPC700

### SuperFX Driver
The SuperFX is a RISC coprocessor used in cartridges like Star Fox. Requires:
1. SuperFX assembly code
2. Memory mapping configuration
3. Frame buffer management
4. Communication with the main CPU
