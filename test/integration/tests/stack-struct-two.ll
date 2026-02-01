; INTEGRATION-TEST
; EXPECT: 30

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test: write to both struct members via function calls
; Simpler than stack-struct-addr - no initialization of second member first

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

  ; Write 10 to first member
  call void @write_first(ptr %s, i16 10)

  ; Write 20 to second member
  call void @write_second(ptr %s, i16 20)

  ; Read both values
  %a = load i16, ptr %s
  %second = getelementptr inbounds %struct.pair, ptr %s, i16 0, i32 1
  %b = load i16, ptr %second
  %sum = add i16 %a, %b
  ret i16 %sum
}
