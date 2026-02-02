; SPC700 Sound Driver for SNES SDK
; ================================
;
; This driver runs on the SPC700 audio processor and provides:
; - Simple tone generation (square wave)
; - Sound effect playback
; - Basic music playback
;
; Communication with the 65816 CPU happens via ports $F4-$F7 (APUIO0-3)
;
; The SPC700 has:
; - 64KB of RAM at $0000-$FFFF
; - 8 DSP voices with BRR sample playback
; - Timer registers
; - I/O ports for CPU communication
;
; Memory Map:
;   $0000-$00EF   Zero page / Direct page
;   $00F0-$00FF   I/O registers
;   $0100-$01FF   Stack page
;   $0200-$02FF   Driver variables
;   $0300-$0FFF   Driver code
;   $1000-$7FFF   Sample data / Music data
;   $8000-$FFFF   Sample directory and BRR samples

; ============================================================================
; SPC700 I/O Registers
; ============================================================================

; Control registers
TEST   = $F0    ; Testing register (write $0A for normal operation)
CONTROL= $F1    ; Control register (timers, I/O, ROM enable)
DSPADDR= $F2    ; DSP register address
DSPDATA= $F3    ; DSP register data

; CPU communication ports (directly mapped to 65816 $2140-$2143)
CPUIO0 = $F4    ; CPU I/O Port 0 (APUIO0 on 65816)
CPUIO1 = $F5    ; CPU I/O Port 1 (APUIO1 on 65816)
CPUIO2 = $F6    ; CPU I/O Port 2 (APUIO2 on 65816)
CPUIO3 = $F7    ; CPU I/O Port 3 (APUIO3 on 65816)

; Timer registers
T0DIV  = $FA    ; Timer 0 divider
T1DIV  = $FB    ; Timer 1 divider
T2DIV  = $FC    ; Timer 2 divider
T0OUT  = $FD    ; Timer 0 output
T1OUT  = $FE    ; Timer 1 output
T2OUT  = $FF    ; Timer 2 output

; ============================================================================
; DSP Registers (accessed via DSPADDR/DSPDATA)
; ============================================================================

; Per-voice registers (voice n = offset $10 * n)
VOLL   = $00    ; Volume left
VOLR   = $01    ; Volume right
PITCHL = $02    ; Pitch low
PITCHH = $03    ; Pitch high
SRCN   = $04    ; Sample source number
ADSR1  = $05    ; ADSR settings 1
ADSR2  = $06    ; ADSR settings 2
GAIN   = $07    ; Gain (alternative to ADSR)
ENVX   = $08    ; Current envelope value (read-only)
OUTX   = $09    ; Current sample output (read-only)

; Global DSP registers
MVOLL  = $0C    ; Master volume left
MVOLR  = $1C    ; Master volume right
EVOLL  = $2C    ; Echo volume left
EVOLR  = $3C    ; Echo volume right
KON    = $4C    ; Key on (start playing voices)
KOFF   = $5C    ; Key off (stop playing voices)
FLG    = $6C    ; Flags (noise, echo, mute, reset)
ENDX   = $7C    ; Voice end flags (read-only)

; Noise and echo registers
EFB    = $0D    ; Echo feedback
PMON   = $2D    ; Pitch modulation enable
NON    = $3D    ; Noise enable
EON    = $4D    ; Echo enable
DIR    = $5D    ; Sample directory address (high byte)
ESA    = $6D    ; Echo buffer start address (high byte)
EDL    = $7D    ; Echo delay (ring buffer size)
FIR    = $0F    ; FIR filter coefficients (0-7 at $0F, $1F, $2F, etc.)

; ============================================================================
; Driver Variables (in SPC700 RAM)
; ============================================================================

.org $0200

; State variables
drv_cmd:        .byte 0     ; Current command from CPU
drv_param:      .byte 0     ; Current parameter from CPU
drv_counter:    .byte 0     ; Command counter for handshaking
master_vol:     .byte 127   ; Master volume (0-127)
sfx_vol:        .byte 127   ; SFX volume (0-127)
music_vol:      .byte 127   ; Music volume (0-127)
current_music:  .byte 0     ; Current music track
sfx_playing:    .byte 0     ; Bit mask of SFX voices in use

; Tone generation
tone_pitch:     .word 0     ; Current tone pitch
tone_duration:  .word 0     ; Frames remaining for tone

; ============================================================================
; Driver Code
; ============================================================================

.org $0300

driver_start:
    ; Initialize stack
    mov X, #$FF
    mov SP, X

    ; Disable timers and enable I/O
    mov CONTROL, #$00

    ; Initialize DSP
    call init_dsp

    ; Signal ready to CPU by writing 0 to port 0
    mov CPUIO0, #$00

    ; Main loop
main_loop:
    ; Check for new command from CPU
    mov A, CPUIO0           ; Read command byte
    cmp A, drv_counter      ; Same as last acknowledged?
    beq main_loop           ; Yes, no new command

    ; New command received - process it
    mov drv_counter, A      ; Acknowledge by echoing counter
    mov CPUIO0, A           ; Echo back to CPU

    ; Extract command and parameter
    mov A, CPUIO0
    lsr A                   ; Shift to get command nibble
    lsr A
    lsr A
    lsr A
    mov drv_cmd, A

    mov A, CPUIO1           ; Get parameter
    mov drv_param, A

    ; Dispatch command
    mov A, drv_cmd
    cmp A, #$01             ; PLAY_SFX
    beq cmd_play_sfx
    cmp A, #$02             ; PLAY_MUSIC
    beq cmd_play_music
    cmp A, #$03             ; STOP_MUSIC
    beq cmd_stop_music
    cmp A, #$04             ; SET_VOLUME
    beq cmd_set_volume
    cmp A, #$05             ; SET_SFX_VOL
    beq cmd_set_sfx_vol
    cmp A, #$06             ; SET_MUS_VOL
    beq cmd_set_mus_vol
    cmp A, #$07             ; STOP_ALL
    beq cmd_stop_all

    ; Unknown command, ignore
    bra main_loop

; ----------------------------------------------------------------------------
; Command handlers
; ----------------------------------------------------------------------------

cmd_play_sfx:
    ; Play sound effect using voice 7 (dedicated SFX voice)
    ; Parameter = SFX ID
    mov A, drv_param
    call play_sfx_tone
    bra main_loop

cmd_play_music:
    ; For now, just store the track ID (actual music not implemented)
    mov A, drv_param
    mov current_music, A
    bra main_loop

cmd_stop_music:
    mov current_music, #$00
    ; Key off music voices (0-6)
    mov A, #$7F
    mov DSPADDR, #KOFF
    mov DSPDATA, A
    bra main_loop

cmd_set_volume:
    mov A, drv_param
    mov master_vol, A
    call update_master_volume
    bra main_loop

cmd_set_sfx_vol:
    mov A, drv_param
    mov sfx_vol, A
    bra main_loop

cmd_set_mus_vol:
    mov A, drv_param
    mov music_vol, A
    bra main_loop

cmd_stop_all:
    ; Key off all voices
    mov A, #$FF
    mov DSPADDR, #KOFF
    mov DSPDATA, A
    mov current_music, #$00
    bra main_loop

; ----------------------------------------------------------------------------
; DSP Initialization
; ----------------------------------------------------------------------------

init_dsp:
    ; Clear DSP flags (disable mute, noise, echo)
    mov DSPADDR, #FLG
    mov DSPDATA, #$00

    ; Set sample directory to $8000
    mov DSPADDR, #DIR
    mov DSPDATA, #$80       ; High byte of $8000

    ; Set master volume
    mov DSPADDR, #MVOLL
    mov DSPDATA, #$7F       ; Max left
    mov DSPADDR, #MVOLR
    mov DSPDATA, #$7F       ; Max right

    ; Disable echo
    mov DSPADDR, #EVOLL
    mov DSPDATA, #$00
    mov DSPADDR, #EVOLR
    mov DSPDATA, #$00
    mov DSPADDR, #EON
    mov DSPDATA, #$00

    ; Initialize voice 7 for SFX (simple square wave)
    mov X, #$70             ; Voice 7 base register

    ; Set volume
    mov DSPADDR, X          ; VOLL
    mov DSPDATA, #$7F
    inc X
    mov DSPADDR, X          ; VOLR
    mov DSPDATA, #$7F

    ; Set default pitch (middle C)
    inc X
    mov DSPADDR, X          ; PITCHL
    mov DSPDATA, #$00
    inc X
    mov DSPADDR, X          ; PITCHH
    mov DSPDATA, #$10

    ; Set sample source 0 (square wave)
    inc X
    mov DSPADDR, X          ; SRCN
    mov DSPDATA, #$00

    ; Set ADSR (short attack, medium decay, no sustain)
    inc X
    mov DSPADDR, X          ; ADSR1
    mov DSPDATA, #$8F       ; Enable ADSR, Attack=15, Decay=0
    inc X
    mov DSPADDR, X          ; ADSR2
    mov DSPDATA, #$E0       ; Sustain=7, Release=0

    ret

; ----------------------------------------------------------------------------
; Play SFX Tone
; A = SFX ID (determines pitch)
; ----------------------------------------------------------------------------

play_sfx_tone:
    push A

    ; Calculate pitch based on SFX ID
    ; Simple mapping: SFX 1=C4, 2=E4, 3=G4, 4=C5, etc.
    mov X, A
    dec X                   ; 0-based index
    bmi @done               ; Invalid SFX ID

    ; Pitch table lookup (16-bit pitch values)
    clrc
    asl A                   ; Multiply by 2 for word index
    mov Y, A
    mov A, pitch_table+Y
    mov DSPADDR, #$72       ; Voice 7 PITCHL
    mov DSPDATA, A
    mov A, pitch_table+1+Y
    mov DSPADDR, #$73       ; Voice 7 PITCHH
    mov DSPDATA, A

    ; Key on voice 7
    mov DSPADDR, #KON
    mov DSPDATA, #$80       ; Voice 7 bit

@done:
    pop A
    ret

; ----------------------------------------------------------------------------
; Update Master Volume
; ----------------------------------------------------------------------------

update_master_volume:
    mov A, master_vol
    mov DSPADDR, #MVOLL
    mov DSPDATA, A
    mov DSPADDR, #MVOLR
    mov DSPDATA, A
    ret

; ============================================================================
; Data Tables
; ============================================================================

; Pitch table for SFX tones (SPC700 pitch format)
; These values produce musical notes at standard frequencies
pitch_table:
    .word $0800             ; C4  (262 Hz) - SFX 1: Beep
    .word $0A00             ; E4  (330 Hz) - SFX 2: Click
    .word $0C00             ; G4  (392 Hz) - SFX 3: Confirm
    .word $0600             ; G3  (196 Hz) - SFX 4: Cancel
    .word $1000             ; C5  (523 Hz) - SFX 5: Jump
    .word $1400             ; E5  (659 Hz) - SFX 6: Coin
    .word $0400             ; C3  (131 Hz) - SFX 7: Hurt
    .word $0000             ; Unused

; ============================================================================
; Sample Directory and BRR Data
; ============================================================================

.org $8000

; Sample directory (4 bytes per entry: start address, loop address)
sample_directory:
    .word square_wave       ; Sample 0: Square wave start
    .word square_wave       ; Sample 0: Loop point

; Simple square wave BRR sample (9 bytes = 16 samples)
; BRR format: 1 header byte + 8 data bytes
; Header: Range(4) Filter(2) Loop(1) End(1)
square_wave:
    .byte $B3               ; Header: Range=11, Filter=0, Loop=1, End=1
    .byte $77, $77, $77, $77 ; High samples (+max)
    .byte $88, $88, $88, $88 ; Low samples (-max)

; ============================================================================
; Reset Vector
; ============================================================================

.org $FFC0

; SPC700 reset vectors
    .word $0300             ; Reset vector -> driver_start
    .word $0300             ; Reserved
