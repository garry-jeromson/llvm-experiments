; INTEGRATION-TEST
; EXPECT: 1

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test boolean OR operation
; Sets bool_a to 0 and bool_b to 1, ORs them, should get 1

@bool_a = global i8 0
@bool_b = global i8 1

define i16 @test_main() {
  ; Load both booleans (as i8 to avoid i1 complexity)
  %a = load i8, ptr @bool_a
  %b = load i8, ptr @bool_b

  ; Zero-extend to i16
  %a16 = zext i8 %a to i16
  %b16 = zext i8 %b to i16

  ; OR them
  %result = or i16 %a16, %b16

  ; Mask to boolean result
  %masked = and i16 %result, 1

  ret i16 %masked
}
