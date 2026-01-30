; INTEGRATION-TEST
; EXPECT: 1

target triple = "w65816-unknown-none"

; 10 > -5 (signed) should be true
define i16 @test_main() {
  %cmp = icmp sgt i16 10, -5
  %result = zext i1 %cmp to i16
  ret i16 %result
}
