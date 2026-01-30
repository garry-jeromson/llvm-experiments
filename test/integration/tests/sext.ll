; INTEGRATION-TEST
; EXPECT: 65408

target triple = "w65816-unknown-none"

; -128 as i8, sign extended to i16 = 0xFF80 = 65408
define i16 @test_main() {
  %narrow = trunc i16 128 to i8
  %result = sext i8 %narrow to i16
  ret i16 %result
}
