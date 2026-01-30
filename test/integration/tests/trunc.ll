; INTEGRATION-TEST
; EXPECT: 171

target triple = "w65816-unknown-none"

; 0x12AB truncated to i8 then zext = 0xAB = 171
define i16 @test_main() {
  %narrow = trunc i16 4779 to i8
  %result = zext i8 %narrow to i16
  ret i16 %result
}
