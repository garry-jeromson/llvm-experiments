; SNES Demo - Direct LLVM IR
; Sets screen to bright green color
;
; This bypasses C and directly uses LLVM IR to demonstrate
; the W65816 backend generating correct SNES code.

target triple = "w65816-unknown-none"

; PPU Register addresses
; INIDISP = $2100 = 8448
; CGADD   = $2121 = 8481
; CGDATA  = $2122 = 8482

define i16 @main() {
entry:
    ; Set CGRAM address to 0 (backdrop color entry)
    ; CGADD = 0
    %cgadd = inttoptr i16 8481 to ptr
    store volatile i8 0, ptr %cgadd, align 1

    ; Write BGR555 color for bright green
    ; Green = 31, so color = (31 << 5) = 0x03E0
    ; Write low byte first: 0xE0 = 224
    %cgdata = inttoptr i16 8482 to ptr
    store volatile i8 -32, ptr %cgdata, align 1

    ; Write high byte: 0x03 = 3
    store volatile i8 3, ptr %cgdata, align 1

    ; Turn screen on: INIDISP = 0x0F (full brightness, no force blank)
    %inidisp = inttoptr i16 8448 to ptr
    store volatile i8 15, ptr %inidisp, align 1

    ret i16 0
}
