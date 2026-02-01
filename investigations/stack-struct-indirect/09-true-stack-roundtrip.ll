; INTEGRATION-TEST
; EXPECT: 42

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Helper that clobbers registers
define void @clobber_regs() {
  ret void
}

; True roundtrip: store to stack, call function (clobbers regs), load back
define i16 @test_main() {
  %x = alloca i16
  store i16 42, ptr %x
  ; Force a reload by calling a function that might clobber registers
  call void @clobber_regs()
  %v = load i16, ptr %x
  ret i16 %v
}
