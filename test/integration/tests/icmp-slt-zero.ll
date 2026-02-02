; INTEGRATION-TEST
; EXPECT: 3

; Test signed less than zero comparison (val < 0)
; LLVM optimizes this to lshr by 15, which we handle efficiently

target triple = "w65816-unknown-none"

define i16 @test_main() {
  %result = add i16 0, 0

  ; Test 1: Positive number (100 < 0 is false)
  %pos = add i16 0, 100
  %cmp_pos = icmp slt i16 %pos, 0
  %pos_result = zext i1 %cmp_pos to i16
  %sum1 = add i16 %result, %pos_result  ; 0 + 0 = 0

  ; Test 2: Negative number (-100 < 0 is true)
  %neg = sub i16 0, 100
  %cmp_neg = icmp slt i16 %neg, 0
  %neg_result = zext i1 %cmp_neg to i16
  %sum2 = add i16 %sum1, %neg_result    ; 0 + 1 = 1

  ; Test 3: Zero (0 < 0 is false)
  %cmp_zero = icmp slt i16 0, 0
  %zero_result = zext i1 %cmp_zero to i16
  %sum3 = add i16 %sum2, %zero_result   ; 1 + 0 = 1

  ; Test 4: Maximum positive (32767 < 0 is false)
  %maxpos = add i16 0, 32767
  %cmp_maxpos = icmp slt i16 %maxpos, 0
  %maxpos_result = zext i1 %cmp_maxpos to i16
  %sum4 = add i16 %sum3, %maxpos_result ; 1 + 0 = 1

  ; Test 5: Minimum negative (-32768 < 0 is true)
  %minneg = add i16 0, 32768
  %cmp_minneg = icmp slt i16 %minneg, 0
  %minneg_result = zext i1 %cmp_minneg to i16
  %sum5 = add i16 %sum4, %minneg_result ; 1 + 1 = 2

  ; Test 6: -1 (0xFFFF < 0 is true)
  %minus1 = sub i16 0, 1
  %cmp_minus1 = icmp slt i16 %minus1, 0
  %minus1_result = zext i1 %cmp_minus1 to i16
  %sum6 = add i16 %sum5, %minus1_result ; 2 + 1 = 3

  ret i16 %sum6
}
