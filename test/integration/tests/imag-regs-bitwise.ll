; INTEGRATION-TEST
; EXPECT: 255

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test bitwise operations with register pressure requiring imaginary registers
; 0xFF00 AND 0x00FF = 0x0000
; 0xF0F0 OR 0x0F0F = 0xFFFF
; 0xAAAA XOR 0x5555 = 0xFFFF
; Result: 0x0000 OR 0xFFFF = 0xFFFF, AND 0x00FF = 0x00FF = 255
define i16 @test_main() {
entry:
  %a = add i16 65280, 0   ; 0xFF00
  %b = add i16 255, 0     ; 0x00FF
  %c = add i16 61680, 0   ; 0xF0F0
  %d = add i16 3855, 0    ; 0x0F0F

  %and1 = and i16 %a, %b  ; 0x0000
  %or1 = or i16 %c, %d    ; 0xFFFF
  %combined = or i16 %and1, %or1  ; 0xFFFF
  %result = and i16 %combined, %b ; 0x00FF = 255
  ret i16 %result
}
