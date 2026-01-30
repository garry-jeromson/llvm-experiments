; INTEGRATION-TEST
; EXPECT: 1

target triple = "w65816-unknown-none"

; -5 < 10 should be true
define i16 @test_main() {
  %cmp = icmp slt i16 -5, 10
  %result = zext i1 %cmp to i16
  ret i16 %result
}
