; INTEGRATION-TEST
; EXPECT: 142

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test: Simple load through pointer
; Verifies basic indirect load functionality

define i16 @test_main() {
entry:
  %arr = alloca [2 x i16]

  ; Store known values
  %p0 = getelementptr [2 x i16], ptr %arr, i16 0, i16 0
  store i16 42, ptr %p0
  %p1 = getelementptr [2 x i16], ptr %arr, i16 0, i16 1
  store i16 100, ptr %p1

  ; Load both values
  %v0 = load i16, ptr %p0   ; 42
  %v1 = load i16, ptr %p1   ; 100

  ; Result: 42 + 100 = 142
  %result = add i16 %v0, %v1
  ret i16 %result
}
