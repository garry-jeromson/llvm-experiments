; INTEGRATION-TEST
; EXPECT: 42
;
; Simple test: Store and load from stack offset 11
; This tests if the emulator handles large stack offsets correctly.

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

define i16 @test_main() {
  ; Force large frame
  %a = alloca i16, align 2
  %b = alloca i16, align 2
  %c = alloca i16, align 2
  %d = alloca i16, align 2
  %e = alloca i16, align 2

  ; Store values
  store i16 1, ptr %a
  store i16 2, ptr %b
  store i16 3, ptr %c
  store i16 4, ptr %d
  store i16 42, ptr %e

  ; Read back the last one
  %val = load i16, ptr %e
  ret i16 %val
}
