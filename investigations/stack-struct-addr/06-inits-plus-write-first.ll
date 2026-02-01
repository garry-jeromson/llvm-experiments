; INTEGRATION-TEST
; EXPECT: 10
;
; Test: Inline inits + write_first only, read first member
; Tests if write_first correctly overwrites the inline init.

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

%struct.pair = type { i16, i16 }

define void @write_first(ptr %p, i16 %val) {
  store i16 %val, ptr %p
  ret void
}

define i16 @test_main() {
  %s = alloca %struct.pair, align 2

  ; Inline init both members
  store i16 11, ptr %s
  %second = getelementptr inbounds %struct.pair, ptr %s, i16 0, i32 1
  store i16 22, ptr %second

  ; Overwrite first member with 10
  call void @write_first(ptr %s, i16 10)

  ; Read first member - should be 10 (from write_first)
  %a = load i16, ptr %s
  ret i16 %a
}
