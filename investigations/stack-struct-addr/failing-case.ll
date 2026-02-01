; INTEGRATION-TEST
; EXPECT: 30
; SKIP: Known failing case - investigating
;
; Failing case: Both inline inits AND both function calls
; Expected: 30 (first=10, second=20)
; Actual: 20 (first=0, second=20)
;
; This is the exact failing case. The first member's value (10) is lost
; and reads back as 0.
;
; The bug ONLY manifests when all of these conditions are met:
; 1. Inline initialization of BOTH struct members
; 2. Calling BOTH write_first AND write_second
; 3. Reading and summing both members

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

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
