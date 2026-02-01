; INTEGRATION-TEST
; EXPECT: 42

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Simplest indirect store - write 42 to *ptr
define void @simple_store(ptr %p) {
  store i16 42, ptr %p
  ret void
}

define i16 @test_main() {
  %x = alloca i16
  store i16 0, ptr %x
  call void @simple_store(ptr %x)
  %v = load i16, ptr %x
  ret i16 %v
}
