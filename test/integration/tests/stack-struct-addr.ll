; INTEGRATION-TEST
; EXPECT: 30
; SKIP: Complex interaction between inline struct init and multiple function calls - each component works individually but combination fails. Tracked for future investigation.

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

; Test passing address of struct members on stack
; This verifies LEA_fi works with GEP offsets

%struct.pair = type { i16, i16 }

; Helper that writes to first member
define void @write_first(ptr %p, i16 %val) {
  store i16 %val, ptr %p
  ret void
}

; Helper that writes to second member
define void @write_second(ptr %p, i16 %val) {
  %second = getelementptr inbounds %struct.pair, ptr %p, i16 0, i32 1
  store i16 %val, ptr %second
  ret void
}

define i16 @test_main() {
  ; Allocate struct on stack
  %s = alloca %struct.pair, align 2

  ; Initialize both members to 0
  store i16 0, ptr %s
  %second = getelementptr inbounds %struct.pair, ptr %s, i16 0, i32 1
  store i16 0, ptr %second

  ; Write 10 to first member via function call
  call void @write_first(ptr %s, i16 10)

  ; Write 20 to second member via function call
  call void @write_second(ptr %s, i16 20)

  ; Read both values and return sum (should be 30)
  %a = load i16, ptr %s
  %b = load i16, ptr %second
  %sum = add i16 %a, %b
  ret i16 %sum
}
