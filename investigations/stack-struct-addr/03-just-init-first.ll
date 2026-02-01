; INTEGRATION-TEST
; EXPECT: 11
;
; Investigation test: Both inline inits, only write_second called
; Result: PASS (11)
;
; This test proves that inline initialization of the first member is preserved
; when only write_second is called. The first member remains 11 as expected.

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

%struct.pair = type { i16, i16 }

define void @write_second(ptr %p, i16 %val) {
  %second = getelementptr inbounds %struct.pair, ptr %p, i16 0, i32 1
  store i16 %val, ptr %second
  ret void
}

define i16 @test_main() {
  %s = alloca %struct.pair, align 2

  ; Initialize first with 11
  store i16 11, ptr %s
  %second = getelementptr inbounds %struct.pair, ptr %s, i16 0, i32 1
  store i16 22, ptr %second

  ; Call write_second (no write to first)
  call void @write_second(ptr %s, i16 54)

  ; Read first (should still be 11)
  %a = load i16, ptr %s
  ret i16 %a
}
