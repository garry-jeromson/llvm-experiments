; INTEGRATION-TEST
; EXPECT: 42

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test direct stack store/load without indirection
; Just allocate on stack, store, and load directly
define i16 @test_main() {
  %x = alloca i16
  store i16 42, ptr %x
  %v = load i16, ptr %x
  ret i16 %v
}
