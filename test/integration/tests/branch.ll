; INTEGRATION-TEST
; EXPECT: 42
; SKIP: Compiler hangs on phi nodes with branches (known issue)

target triple = "w65816-unknown-none"

define i16 @test_main() {
entry:
  %cmp = icmp eq i16 1, 1
  br i1 %cmp, label %then, label %else

then:
  br label %merge

else:
  br label %merge

merge:
  %result = phi i16 [ 42, %then ], [ 99, %else ]
  ret i16 %result
}
