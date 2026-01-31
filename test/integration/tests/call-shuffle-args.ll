; INTEGRATION-TEST
; EXPECT: 6

target triple = "w65816-unknown-none"

; Helper that returns a - b + c (to verify argument order matters)
define i16 @combine(i16 %a, i16 %b, i16 %c) {
  %t1 = sub i16 %a, %b
  %result = add i16 %t1, %c
  ret i16 %result
}

; Wrapper that shuffles arguments: receives (a,b,c) calls combine(c,b,a)
; If registers aren't shuffled correctly, we get wrong result
; combine(c,b,a) = c - b + a
; With a=1, b=2, c=3: combine(3,2,1) = 3 - 2 + 1 = 2
; But if shuffle doesn't work: combine(1,2,3) = 1 - 2 + 3 = 2
; Need different values to detect the bug...
; Let's try: a=10, b=3, c=1
; combine(c,b,a) = combine(1,3,10) = 1 - 3 + 10 = 8
; combine(a,b,c) = combine(10,3,1) = 10 - 3 + 1 = 8
; Still same! Need asymmetric formula.
; Let's use: a + b - c
define i16 @combine2(i16 %a, i16 %b, i16 %c) {
  %t1 = add i16 %a, %b
  %result = sub i16 %t1, %c
  ret i16 %result
}

; Wrapper that shuffles: (a,b,c) -> (c,b,a)
; combine2(c,b,a) = c + b - a
define i16 @shuffle(i16 %a, i16 %b, i16 %c) {
  %result = call i16 @combine2(i16 %c, i16 %b, i16 %a)
  ret i16 %result
}

; shuffle(1, 2, 3) calls combine2(3, 2, 1) = 3 + 2 - 1 = 4
; If args NOT shuffled: combine2(1, 2, 3) = 1 + 2 - 3 = 0
define i16 @test_main() {
  %result = call i16 @shuffle(i16 1, i16 2, i16 3)
  ; Add 2 to distinguish from 0 result
  %final = add i16 %result, 2
  ret i16 %final  ; Expected: 4 + 2 = 6
}
