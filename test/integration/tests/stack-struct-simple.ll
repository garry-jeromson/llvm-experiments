; INTEGRATION-TEST
; EXPECT: 10

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Simplified test: just write to first member and read back
; No second member, no GEP offsets

define void @write_value(ptr %p, i16 %val) {
  store i16 %val, ptr %p
  ret void
}

define i16 @test_main() {
  %s = alloca i16, align 2
  store i16 0, ptr %s
  call void @write_value(ptr %s, i16 10)
  %a = load i16, ptr %s
  ret i16 %a
}
