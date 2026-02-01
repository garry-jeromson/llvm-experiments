; INTEGRATION-TEST
; EXPECT: 11
;
; Test: Only inline inits, no function calls
; If this passes, inline init is working correctly.

target datalayout = "e-m:e-p:16:16-i8:8-i16:16-n8:16-S16"
target triple = "w65816-unknown-none"

%struct.pair = type { i16, i16 }

define i16 @test_main() {
  %s = alloca %struct.pair, align 2

  ; Inline init both members to specific values
  store i16 11, ptr %s
  %second = getelementptr inbounds %struct.pair, ptr %s, i16 0, i32 1
  store i16 22, ptr %second

  ; Read first member only
  %a = load i16, ptr %s
  ret i16 %a
}
