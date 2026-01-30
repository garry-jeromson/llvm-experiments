; W65816 Integration Test Wrapper
;
; This wrapper:
; 1. Sets up 16-bit native mode
; 2. Initializes stack pointer
; 3. Calls the test function (test_main)
; 4. Stores result from A register to $0000
; 5. Halts CPU with STP instruction

.p816           ; Enable 65816 mode
.smart          ; Smart mode for automatic sizing

.segment "CODE"

; Import the test function - LLVM generates this with underscore prefix
.import _test_main

; Export start symbol
.export _start

_start:
    clc             ; Clear carry
    xce             ; Switch to native mode

    rep #$30        ; 16-bit accumulator and index registers
    .a16
    .i16

    ldx #$01FF      ; Initialize stack pointer
    txs

    jsr _test_main  ; Call test function, result in A

    sta $0000       ; Store result to memory location 0

    stp             ; Stop CPU (signals test completion)

; Vectors segment - reset vector points to _start
.segment "VECTORS"
    .word 0         ; $FFFA: NMI vector (unused)
    .word _start    ; $FFFC: RESET vector
    .word 0         ; $FFFE: IRQ/BRK vector (unused)
