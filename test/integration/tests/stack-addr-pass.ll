; INTEGRATION-TEST
; EXPECT: 42

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test passing a stack address to a function
; This verifies that LEA_fi correctly computes the stack address

; Helper function that writes a value through a pointer
define void @write_value(ptr %p, i16 %val) {
  store i16 %val, ptr %p
  ret void
}

define i16 @test_main() {
  ; Allocate a variable on the stack
  %var = alloca i16, align 2

  ; Initialize to 0
  store i16 0, ptr %var

  ; Pass the address to a function that writes 42 to it
  call void @write_value(ptr %var, i16 42)

  ; Read the value back - should be 42
  %result = load i16, ptr %var
  ret i16 %result
}
