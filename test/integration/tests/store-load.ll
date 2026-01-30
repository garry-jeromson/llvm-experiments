; INTEGRATION-TEST
; EXPECT: 999

target triple = "w65816-unknown-none"

@scratch = global i16 0

define i16 @test_main() {
  store i16 999, ptr @scratch
  %val = load i16, ptr @scratch
  ret i16 %val
}
