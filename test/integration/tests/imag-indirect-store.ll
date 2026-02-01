; INTEGRATION-TEST
; EXPECT: 75

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test: Store through pointer and read back
; Verifies basic indirect store functionality

define void @write_val(ptr %p, i16 %val) {
entry:
  store i16 %val, ptr %p
  ret void
}

define i16 @test_main() {
entry:
  %x = alloca i16

  ; Store value via function call
  call void @write_val(ptr %x, i16 75)

  ; Load result back
  %result = load i16, ptr %x
  ret i16 %result
}
