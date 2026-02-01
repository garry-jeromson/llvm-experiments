; INTEGRATION-TEST
; EXPECT: 42

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Store at offset 2 from pointer (like write_second does)
define void @store_at_offset2(ptr %p, i16 %val) {
  %offset = getelementptr i8, ptr %p, i16 2
  store i16 %val, ptr %offset
  ret void
}

define i16 @test_main() {
  %arr = alloca [2 x i16]  ; 4 bytes
  ; Store at offset 2 (second element)
  call void @store_at_offset2(ptr %arr, i16 42)
  ; Read from offset 2
  %offset = getelementptr i8, ptr %arr, i16 2
  %v = load i16, ptr %offset
  ret i16 %v
}
