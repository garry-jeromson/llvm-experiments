; INTEGRATION-TEST
; EXPECT: 20

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test direct store/load to second element without function call
define i16 @test_main() {
  %arr = alloca [2 x i16]

  ; Store to first element
  %first = getelementptr [2 x i16], ptr %arr, i16 0, i16 0
  store i16 10, ptr %first

  ; Store to second element
  %second = getelementptr [2 x i16], ptr %arr, i16 0, i16 1
  store i16 20, ptr %second

  ; Load second element
  %v = load i16, ptr %second
  ret i16 %v
}
