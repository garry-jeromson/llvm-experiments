; INTEGRATION-TEST
; EXPECT: 255

target triple = "w65816-unknown-none"

; 255 as i8, zero extended to i16 = 255
define i16 @test_main() {
  %narrow = trunc i16 255 to i8
  %result = zext i8 %narrow to i16
  ret i16 %result
}
