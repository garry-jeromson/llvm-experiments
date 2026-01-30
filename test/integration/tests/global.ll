; INTEGRATION-TEST
; EXPECT: 1234

target triple = "w65816-unknown-none"

@global_val = global i16 1234

define i16 @test_main() {
  %val = load i16, ptr @global_val
  ret i16 %val
}
