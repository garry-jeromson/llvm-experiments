; INTEGRATION-TEST
; EXPECT: 30

; Test that shift results work correctly when placed in non-A registers.
; Computes a shifted value and adds it to a base, verifying the shift
; produces the correct result regardless of which register holds it.
; 10 + (5 << 2) = 10 + 20 = 30

target datalayout = "e-m:e-p:16:16-i16:16-n8:16"
target triple = "w65816-unknown-none"

define i16 @compute(i16 %base, i16 %val) {
  %shifted = shl i16 %val, 2
  %result = add i16 %base, %shifted
  ret i16 %result
}

define i16 @test_main() {
  %r = call i16 @compute(i16 10, i16 5)
  ret i16 %r
}
