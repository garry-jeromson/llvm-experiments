; INTEGRATION-TEST
; EXPECT: 3
; Test signed comparison with immediate values where V flag matters
; Uses volatile loads to prevent constant folding

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

@neg1 = global i16 -1
@max_pos = global i16 32767
@min_neg = global i16 -32768
@zero = global i16 0

; Test signed less-than with edge cases involving overflow:
; 1. -1 < 32767 (true) → +1
; 2. -32768 < 0 (true) → +1
; 3. -1 < 0 (true) → +1
; Total: 3

define i16 @test_main() {
entry:
  ; Load values via volatile-like globals to prevent constant folding
  %a = load volatile i16, ptr @neg1
  %b = load volatile i16, ptr @max_pos
  %c = load volatile i16, ptr @min_neg
  %d = load volatile i16, ptr @zero

  ; Test 1: -1 < 32767 (signed) → true → 1
  %cmp1 = icmp slt i16 %a, %b
  %v1 = zext i1 %cmp1 to i16

  ; Test 2: -32768 < 0 (signed) → true → 1
  %cmp2 = icmp slt i16 %c, %d
  %v2 = zext i1 %cmp2 to i16

  ; Test 3: -1 < 0 (signed) → true → 1
  %cmp3 = icmp slt i16 %a, %d
  %v3 = zext i1 %cmp3 to i16

  ; Sum: 1 + 1 + 1 = 3
  %sum1 = add i16 %v1, %v2
  %sum2 = add i16 %sum1, %v3

  ret i16 %sum2
}
