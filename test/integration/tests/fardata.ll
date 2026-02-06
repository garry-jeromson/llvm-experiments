; INTEGRATION-TEST
; EXPECT: 142
; Test 24-bit long addressing for far data sections
; This test verifies that globals in .fardata sections are accessed correctly

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Far data section globals - should use long addressing
@far_value = global i16 100, section ".fardata"
@far_addend = global i16 42, section ".fardata"

; Normal bank 0 global for comparison
@normal_value = global i16 50

define i16 @test_main() {
  ; Load from far data section
  %a = load i16, ptr @far_value
  %b = load i16, ptr @far_addend

  ; Add them together
  %sum = add i16 %a, %b

  ; Result should be 100 + 42 = 142
  ret i16 %sum
}
