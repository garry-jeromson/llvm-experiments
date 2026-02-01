; INTEGRATION-TEST
; EXPECT: 10
;
; Minimal test: Large frame, init both, write_first only, read first
; This should isolate whether the issue is with write_first or write_second

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

%struct.pair = type { i16, i16 }

define void @write_first(ptr %p, i16 %val) {
  store i16 %val, ptr %p
  ret void
}

define i16 @test_main() {
  %s = alloca %struct.pair, align 2
  ; Extra allocas to force larger frame
  %extra1 = alloca i16, align 2
  %extra2 = alloca i16, align 2

  ; Use the extra allocas
  store i16 99, ptr %extra1
  store i16 98, ptr %extra2

  ; Inline init both members
  store i16 11, ptr %s
  %second = getelementptr inbounds %struct.pair, ptr %s, i16 0, i32 1
  store i16 22, ptr %second

  ; Only write_first
  call void @write_first(ptr %s, i16 10)

  ; Read first member
  %a = load i16, ptr %s
  ret i16 %a
}
