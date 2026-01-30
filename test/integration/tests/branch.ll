; INTEGRATION-TEST
; EXPECT: 42

target triple = "w65816-unknown-none"

; Test conditional branch using select (phi nodes have codegen issues)
define i16 @branch_test(i16 %x) {
entry:
  %cmp = icmp eq i16 %x, 0
  %result = select i1 %cmp, i16 42, i16 99
  ret i16 %result
}

; Entry point calls branch_test with 0
define i16 @test_main() {
  %result = call i16 @branch_test(i16 0)
  ret i16 %result
}
