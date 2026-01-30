; INTEGRATION-TEST
; EXPECT: 100

target triple = "w65816-unknown-none"

define i16 @test_main() {
  %cmp = icmp sgt i16 50, 25
  %result = select i1 %cmp, i16 100, i16 200
  ret i16 %result
}
