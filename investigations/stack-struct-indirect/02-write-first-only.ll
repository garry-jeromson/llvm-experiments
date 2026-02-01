; INTEGRATION-TEST
; EXPECT: 10

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

%struct.pair = type { i16, i16 }

define void @write_first(ptr %p, i16 %val) {
  store i16 %val, ptr %p
  ret void
}

define i16 @test_main() {
  %s = alloca %struct.pair, align 2
  call void @write_first(ptr %s, i16 10)
  %a = load i16, ptr %s
  ret i16 %a
}
