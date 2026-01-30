; INTEGRATION-TEST
; EXPECT: 55

target triple = "w65816-unknown-none"

; Sum 1 to 10 = 55
define i16 @test_main() {
entry:
  br label %loop

loop:
  %i = phi i16 [ 1, %entry ], [ %next_i, %loop ]
  %sum = phi i16 [ 0, %entry ], [ %next_sum, %loop ]
  %next_sum = add i16 %sum, %i
  %next_i = add i16 %i, 1
  %done = icmp ugt i16 %next_i, 10
  br i1 %done, label %exit, label %loop

exit:
  ret i16 %next_sum
}
