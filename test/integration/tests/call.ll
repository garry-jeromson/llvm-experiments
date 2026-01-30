; INTEGRATION-TEST
; EXPECT: 15

target triple = "w65816-unknown-none"

define i16 @add_nums(i16 %a, i16 %b) {
  %sum = add i16 %a, %b
  ret i16 %sum
}

define i16 @test_main() {
  %result = call i16 @add_nums(i16 7, i16 8)
  ret i16 %result
}
