; INTEGRATION-TEST
; EXPECT: 1

target triple = "w65816-unknown-none"

; Simple unsigned comparison without function call
; Uses add i16 X, 0 to force runtime computation
define i16 @test_main() {
entry:
  %a = add i16 10, 0
  %b = add i16 20, 0
  %cmp = icmp ult i16 %a, %b
  br i1 %cmp, label %then, label %else
then:
  ret i16 1
else:
  ret i16 0
}
