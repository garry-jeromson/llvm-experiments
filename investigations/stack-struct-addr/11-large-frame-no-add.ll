; INTEGRATION-TEST
; EXPECT: 10
;
; Test: Large frame, read both, return first (no add)
; Force a large frame by adding extra allocas to see if the read is affected.

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
  ; Extra allocas to force larger frame
  %extra1 = alloca i16, align 2
  %extra2 = alloca i16, align 2
  %extra3 = alloca i16, align 2

  ; Use the extra allocas to prevent them from being optimized away
  store i16 99, ptr %extra1
  store i16 98, ptr %extra2
  store i16 97, ptr %extra3

  ; Inline init both members
  store i16 11, ptr %s
  %second = getelementptr inbounds %struct.pair, ptr %s, i16 0, i32 1
  store i16 22, ptr %second

  ; Overwrite both members
  call void @write_first(ptr %s, i16 10)
  call void @write_second(ptr %s, i16 20)

  ; Read both but return only first
  %a = load i16, ptr %s
  %b = load i16, ptr %second

  ret i16 %a
}
