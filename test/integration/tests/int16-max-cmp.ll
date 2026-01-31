; INTEGRATION-TEST
; EXPECT: 1

target triple = "w65816-unknown-none"

; Test comparisons with INT16_MAX (32767 = 0x7FFF)
; This is the boundary between positive numbers and negative (in signed representation)

define i16 @is_max_value(i16 %x) {
  %cmp = icmp eq i16 %x, 32767
  %result = zext i1 %cmp to i16
  ret i16 %result
}

; 32767 should equal INT16_MAX, return 1
define i16 @test_main() {
  %result = call i16 @is_max_value(i16 32767)
  ret i16 %result
}
