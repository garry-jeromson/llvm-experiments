; INTEGRATION-TEST
; EXPECT: 1

target triple = "w65816-unknown-none"

; Test comparisons with edge case values
; INT16_MIN = -32768 = 0x8000
; INT16_MAX = 32767 = 0x7FFF

define i16 @is_negative(i16 %x) {
  %cmp = icmp slt i16 %x, 0
  %result = zext i1 %cmp to i16
  ret i16 %result
}

; INT16_MIN (-32768) is negative, should return 1
define i16 @test_main() {
  %result = call i16 @is_negative(i16 -32768)
  ret i16 %result
}
