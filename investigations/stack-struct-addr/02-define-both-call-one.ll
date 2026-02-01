; INTEGRATION-TEST
; EXPECT: 33
;
; Investigation test: Both inline inits, only write_first called
; Result: PASS (33)
;
; This test proves that inline initialization works correctly when only
; one function (write_first) is called, even though write_second is defined.
; The presence of write_second in the module is not sufficient to trigger the bug.

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

%struct.pair = type { i16, i16 }

define void @write_first(ptr %p, i16 %val) {
  store i16 %val, ptr %p
  ret void
}

define void @write_second(ptr %p, i16 %val) {
  %second = getelementptr inbounds %struct.pair, ptr %p, i16 0, i32 1
  store i16 %val, ptr %second
  ret void
}

define i16 @test_main() {
  %s = alloca %struct.pair, align 2

  ; Initialize both members
  store i16 11, ptr %s
  %second = getelementptr inbounds %struct.pair, ptr %s, i16 0, i32 1
  store i16 22, ptr %second

  ; ONLY call write_first (but write_second is defined)
  call void @write_first(ptr %s, i16 33)

  %a = load i16, ptr %s
  ret i16 %a
}
