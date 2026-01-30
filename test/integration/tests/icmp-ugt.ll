; INTEGRATION-TEST
; EXPECT: 1

target triple = "w65816-unknown-none"

; 65531 > 10 (unsigned) should be true
define i16 @test_main() {
  %cmp = icmp ugt i16 65531, 10
  %result = zext i1 %cmp to i16
  ret i16 %result
}
