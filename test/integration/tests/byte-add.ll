; INTEGRATION-TEST
; EXPECT: 8

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test adding two byte values (8-bit loads zero-extended)
; 5 + 3 = 8

@byte_a = global i8 5
@byte_b = global i8 3

define i16 @test_main() {
  %a = load i8, ptr @byte_a
  %b = load i8, ptr @byte_b
  %a16 = zext i8 %a to i16
  %b16 = zext i8 %b to i16
  %result = add i16 %a16, %b16
  ret i16 %result
}
