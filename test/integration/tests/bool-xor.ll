; INTEGRATION-TEST
; EXPECT: 0

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test boolean XOR operation
; Sets both booleans to 1, XORs them, should get 0 (1 XOR 1 = 0)

@bool_a = global i8 1
@bool_b = global i8 1

define i16 @test_main() {
  ; Load both booleans (as i8 to avoid i1 complexity)
  %a = load i8, ptr @bool_a
  %b = load i8, ptr @bool_b

  ; Zero-extend to i16
  %a16 = zext i8 %a to i16
  %b16 = zext i8 %b to i16

  ; XOR them
  %result = xor i16 %a16, %b16

  ; Mask to boolean result
  %masked = and i16 %result, 1

  ret i16 %masked
}
