; INTEGRATION-TEST
; EXPECT: 0

target triple = "w65816-unknown-none"

; 65531 (unsigned) < 10 should be false
; (65531 = -5 as signed, but interpreted as unsigned)
define i16 @test_main() {
  %cmp = icmp ult i16 65531, 10
  %result = zext i1 %cmp to i16
  ret i16 %result
}
