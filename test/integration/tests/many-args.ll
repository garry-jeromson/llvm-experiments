; INTEGRATION-TEST
; EXPECT: 21

target triple = "w65816-unknown-none"

; Test function with more than 3 arguments (requires stack passing)
; Args: A=a, X=b, Y=c, stack=d,e,f
define i16 @sum6(i16 %a, i16 %b, i16 %c, i16 %d, i16 %e, i16 %f) {
  %ab = add i16 %a, %b
  %abc = add i16 %ab, %c
  %abcd = add i16 %abc, %d
  %abcde = add i16 %abcd, %e
  %result = add i16 %abcde, %f
  ret i16 %result
}

; sum6(1,2,3,4,5,6) = 21
define i16 @test_main() {
  %result = call i16 @sum6(i16 1, i16 2, i16 3, i16 4, i16 5, i16 6)
  ret i16 %result
}
