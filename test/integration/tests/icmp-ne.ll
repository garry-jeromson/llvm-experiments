; INTEGRATION-TEST
; EXPECT: 1

target triple = "w65816-unknown-none"

define i16 @test_main() {
  %cmp = icmp ne i16 42, 43
  %result = zext i1 %cmp to i16
  ret i16 %result
}
