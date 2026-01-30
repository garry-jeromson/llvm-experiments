; INTEGRATION-TEST
; EXPECT: 32

target triple = "w65816-unknown-none"

; Test nested function calls
define i16 @double(i16 %x) {
  %result = shl i16 %x, 1
  ret i16 %result
}

define i16 @quadruple(i16 %x) {
  %d = call i16 @double(i16 %x)
  %result = call i16 @double(i16 %d)
  ret i16 %result
}

; quadruple(8) = double(double(8)) = double(16) = 32
define i16 @test_main() {
  %result = call i16 @quadruple(i16 8)
  ret i16 %result
}
