; SNES SDK - C API Implementation (Assembly)
; Direct hardware access for W65816 target
; Hand-written to avoid register pressure issues

.p816
.smart
.a16
.i16

; ============================================================================
; Hardware Register Definitions
; ============================================================================

INIDISP  = $2100
OBSEL    = $2101
OAMADDL  = $2102
OAMADDH  = $2103
OAMDATA  = $2104
CGADD    = $2121
CGDATA   = $2122
TM       = $212C
HVBJOY   = $4212
JOY1L    = $4218
JOY1H    = $4219
NMITIMEN = $4200

; ============================================================================
; RAM Variables (BSS segment)
; ============================================================================

.segment "BSS"

oam_low:     .res 512    ; 128 sprites * 4 bytes
oam_high:    .res 32     ; High bits for X position and size
joy_current: .res 4      ; Current joypad state (2 pads * 2 bytes)
joy_previous:.res 4      ; Previous joypad state

; ============================================================================
; Code Segment
; ============================================================================

.segment "CODE"

; Export all API symbols
.export snes_init
.export snes_screen_off
.export snes_screen_on
.export snes_set_bgcolor_rgb
.export snes_sprites_set_obsel
.export snes_set_main_screen
.export snes_wait_vblank
.export snes_joy_update
.export snes_joy_held
.export snes_joy_pressed
.export snes_clamp
.export snes_sprite_set_pos
.export snes_sprite_set_tile
.export snes_sprite_hide
.export snes_sprites_upload
.export snes_dma_vram
.export snes_set_sprite_palette

; Import sprite data from sprites.s
.import sprite_data, sprite_data_size

; ============================================================================
; snes_init - Initialize SDK
; ============================================================================
.proc snes_init
    ; Clear OAM low table
    ldx #0
@clear_oam_low:
    stz oam_low,x
    inx
    inx
    cpx #512
    bne @clear_oam_low

    ; Clear OAM high table
    ldx #0
@clear_oam_high:
    stz oam_high,x
    inx
    inx
    cpx #32
    bne @clear_oam_high

    ; Hide all sprites (Y = 240 = off-screen)
    ldx #0
    lda #240
@hide_sprites:
    sep #$20            ; 8-bit accumulator
    sta oam_low+1,x     ; Y position
    rep #$20            ; 16-bit accumulator
    inx
    inx
    inx
    inx
    cpx #512
    bne @hide_sprites

    ; Clear joypad state
    stz joy_current
    stz joy_current+2
    stz joy_previous
    stz joy_previous+2

    ; Enable joypad auto-read
    sep #$20
    lda #$01
    sta NMITIMEN
    rep #$20

    ; Upload shadow OAM to PPU immediately
    ; This ensures sprites are hidden before screen turns on
    jsr snes_sprites_upload

    rts
.endproc

; ============================================================================
; snes_screen_off - Turn off display (force blank)
; ============================================================================
.proc snes_screen_off
    sep #$20
    lda #$80
    sta INIDISP
    rep #$20
    rts
.endproc

; ============================================================================
; snes_screen_on - Turn on display with brightness
; Args: A = brightness (0-15)
; ============================================================================
.proc snes_screen_on
    cmp #16
    bcc @ok
    lda #15
@ok:
    sep #$20
    sta INIDISP
    rep #$20
    rts
.endproc

; ============================================================================
; snes_set_bgcolor_rgb - Set background color
; Args: A = r (0-31), X = g (0-31), Y = b (0-31)
; NOTE: Uses only stack for temps, preserves all DP locations
; ============================================================================
.proc snes_set_bgcolor_rgb
    ; Save params on stack
    pha                 ; r
    phx                 ; g
    phy                 ; b

    ; Clamp r to 31
    lda 5,s             ; r
    cmp #32
    bcc @r_ok
    lda #31
@r_ok:
    sta 5,s             ; Save clamped r

    ; Clamp g to 31
    lda 3,s             ; g
    cmp #32
    bcc @g_ok
    lda #31
@g_ok:
    asl a               ; g << 5
    asl a
    asl a
    asl a
    asl a
    ora 5,s             ; Combine with r
    sta 5,s             ; Save r|g

    ; Clamp b to 31
    lda 1,s             ; b
    cmp #32
    bcc @b_ok
    lda #31
@b_ok:
    ; b << 10
    xba                 ; Swap bytes: b now in high byte
    asl a               ; Shift left 2 more
    asl a
    and #$7C00          ; Mask to bits 10-14
    ora 5,s             ; Combine with r|g

    ; Write to CGRAM
    sep #$20
    stz CGADD           ; Address 0
    sta CGDATA          ; Low byte
    xba
    sta CGDATA          ; High byte
    rep #$20

    ; Clean up stack
    pla
    pla
    pla
    rts
.endproc

; ============================================================================
; snes_sprites_set_obsel - Configure sprite base address and size mode
; Args: A = base_addr (VRAM word address), X = size_mode (0-7)
; NOTE: Uses only stack for temps, preserves all DP locations
; ============================================================================
.proc snes_sprites_set_obsel
    ; Save params on stack
    pha                 ; base_addr
    phx                 ; size_mode

    ; base_addr >> 13 gives the 3-bit value
    lda 3,s             ; base_addr
    lsr a
    lsr a
    lsr a
    lsr a               ; >> 4
    lsr a
    lsr a
    lsr a
    lsr a               ; >> 8
    lsr a
    lsr a
    lsr a
    lsr a
    lsr a               ; >> 13
    and #$07
    sta 3,s             ; Store base_bits in stack slot

    ; Add size_mode << 5
    lda 1,s             ; size_mode
    and #$07
    asl a
    asl a
    asl a
    asl a
    asl a               ; << 5
    ora 3,s             ; Combine with base_bits

    sep #$20
    sta OBSEL
    rep #$20

    ; Clean up stack
    pla
    pla
    rts
.endproc

; ============================================================================
; snes_set_main_screen - Enable layers on main screen
; Args: A = layer_mask
; ============================================================================
.proc snes_set_main_screen
    sep #$20
    sta TM
    rep #$20
    rts
.endproc

; ============================================================================
; snes_wait_vblank - Wait for vertical blank
; Returns DURING vblank so OAM/VRAM updates are safe
; ============================================================================
.proc snes_wait_vblank
    ; First, if we're currently in vblank, wait for it to end
    ; This ensures we don't return immediately on the same vblank
@wait_end:
    sep #$20
    lda HVBJOY
    rep #$20
    and #$80
    bne @wait_end       ; Loop while IN vblank (bit 7 set)

    ; Now wait for the NEXT vblank to start
@wait_start:
    sep #$20
    lda HVBJOY
    rep #$20
    and #$80
    beq @wait_start     ; Loop while NOT in vblank (bit 7 clear)

    ; We're now in vblank - safe to update OAM/VRAM
    rts
.endproc

; ============================================================================
; snes_joy_update - Update joypad state
; ============================================================================
.proc snes_joy_update
    ; Wait for auto-read to complete
@wait:
    sep #$20
    lda HVBJOY
    rep #$20
    and #$01
    bne @wait

    ; Save previous state
    lda joy_current
    sta joy_previous
    lda joy_current+2
    sta joy_previous+2

    ; Read new state
    sep #$20
    lda JOY1L
    xba
    lda JOY1H
    xba                 ; A now has JOY1L in low byte, JOY1H will be read
    rep #$20
    ; Re-read properly
    sep #$20
    lda JOY1L
    sta joy_current
    lda JOY1H
    sta joy_current+1
    rep #$20

    rts
.endproc

; ============================================================================
; snes_joy_held - Check if button is held
; Args: A = joypad_id (0-1), X = button_mask
; Returns: A = 1 if held, 0 if not
; ============================================================================
.proc snes_joy_held
    cmp #2
    bcs @invalid

    ; Save DP and use stack for temp storage
    pha                 ; Save joypad_id
    phx                 ; Save button_mask

    ; Get joypad_id back and calculate index
    lda 3,s             ; Load joypad_id from stack
    asl a               ; * 2 for 16-bit values
    tay
    lda joy_current,y   ; Load current joypad state
    and 1,s             ; AND with button_mask on stack
    beq @not_held

    ; Clean up stack and return 1
    pla
    pla
    lda #1
    rts

@not_held:
    pla
    pla
    lda #0
    rts

@invalid:
    lda #0
    rts
.endproc

; ============================================================================
; snes_joy_pressed - Check if button was just pressed
; Args: A = joypad_id (0-1), X = button_mask
; Returns: A = 1 if just pressed, 0 if not
; NOTE: Uses only stack for temps, preserves all DP locations
; ============================================================================
.proc snes_joy_pressed
    cmp #2
    bcs @invalid

    ; Save params on stack
    pha                 ; joypad_id
    phx                 ; button_mask

    ; Get index into arrays
    lda 3,s             ; joypad_id
    asl a
    tay

    ; Check current & mask
    lda joy_current,y
    and 1,s             ; button_mask
    beq @not_pressed    ; Not held now

    ; Check previous & mask
    lda joy_previous,y
    and 1,s             ; button_mask
    bne @not_pressed    ; Was held before too

    pla
    pla
    lda #1
    rts

@not_pressed:
    pla
    pla
@invalid:
    lda #0
    rts
.endproc

; ============================================================================
; snes_clamp - Clamp value to range
; Args: A = value, X = lo, Y = hi
; Returns: A = clamped value
; NOTE: Uses only stack for temps, preserves all DP locations
; ============================================================================
.proc snes_clamp
    ; Save params on stack
    pha                 ; value
    phx                 ; lo
    phy                 ; hi

    ; Compare value with lo (signed)
    lda 5,s             ; value
    sec
    sbc 3,s             ; value - lo
    bvc @no_overflow1
    eor #$8000          ; Fix sign after overflow
@no_overflow1:
    bpl @check_hi       ; value >= lo (signed)

    ; value < lo, return lo
    pla
    pla
    pla
    txa                 ; Return lo (still in X)
    rts

@check_hi:
    ; Compare value with hi (signed)
    lda 5,s             ; value
    sec
    sbc 1,s             ; value - hi
    bvc @no_overflow2
    eor #$8000
@no_overflow2:
    bmi @return_val     ; value < hi
    beq @return_val     ; value == hi

    ; value > hi, return hi
    pla
    pla
    pla
    tya                 ; Return hi (still in Y)
    rts

@return_val:
    lda 5,s             ; Return original value
    ply
    plx
    plx                 ; Pop the saved value
    rts
.endproc

; ============================================================================
; snes_sprite_set_pos - Set sprite position
; Args: A = id, X = x position (signed 16-bit), Y = y position (8-bit in low byte)
; NOTE: Uses only stack for temps, preserves all DP locations
; ============================================================================
.proc snes_sprite_set_pos
    cmp #128
    bcs @invalid

    ; Save params on stack (don't use DP!)
    pha                 ; [1,s] = id
    phx                 ; [1,s] = x, [3,s] = id
    phy                 ; [1,s] = y, [3,s] = x, [5,s] = id

    ; Calculate OAM index = id * 4
    lda 5,s             ; id
    asl a
    asl a
    tax                 ; X = index into oam_low

    ; Store X low byte
    lda 3,s             ; x position
    sep #$20
    sta oam_low,x
    rep #$20

    ; Store Y
    lda 1,s             ; y position
    sep #$20
    sta oam_low+1,x
    rep #$20

    ; Set X high bit in high table
    ; byte_idx = id >> 2
    lda 5,s             ; id
    lsr a
    lsr a               ; id >> 2
    tay                 ; Y = byte_idx

    ; bit_pos = (id & 3) << 1
    lda 5,s             ; id
    and #$03
    asl a
    tax                 ; X = bit_pos

    ; Check if x position & $100 (bit 8 set)
    lda 3,s             ; x position
    and #$100
    beq @clear_bit

    ; Set bit: oam_high[byte_idx] |= (1 << bit_pos)
    lda #1
@shift_set:
    cpx #0
    beq @do_set
    asl a
    dex
    bra @shift_set
@do_set:
    sep #$20
    ora oam_high,y
    sta oam_high,y
    rep #$20
    bra @done

@clear_bit:
    ; Clear bit: oam_high[byte_idx] &= ~(1 << bit_pos)
    lda 5,s             ; id
    and #$03
    asl a               ; bit_pos
    tax
    lda #1
@shift_clear:
    cpx #0
    beq @do_clear
    asl a
    dex
    bra @shift_clear
@do_clear:
    eor #$FF            ; Invert for AND mask
    sep #$20
    and oam_high,y
    sta oam_high,y
    rep #$20

@done:
    ; Clean up stack
    pla
    pla
    pla
    rts

@invalid:
    rts
.endproc

; ============================================================================
; snes_sprite_set_tile - Set sprite tile and attributes
; C ABI: A = id, X = tile, Y = palette (hflip/vflip ignored for simplicity)
; NOTE: Uses only stack for temps, preserves all DP locations
; ============================================================================
.proc snes_sprite_set_tile
    cmp #128
    bcs @invalid

    ; Save params on stack
    pha                 ; id
    phx                 ; tile
    phy                 ; palette

    ; Calculate OAM index = id * 4
    lda 5,s             ; id
    asl a
    asl a
    tax                 ; X = index into oam_low

    ; Store tile low byte
    lda 3,s             ; tile
    sep #$20
    sta oam_low+2,x
    rep #$20

    ; Build attribute byte: (tile >> 8) & 1 | (palette & 7) << 1
    lda 3,s             ; tile
    xba                 ; Get high byte into low
    and #$01            ; Name table select bit
    sta 5,s             ; Reuse id slot for attr temp

    lda 1,s             ; palette
    and #$07
    asl a               ; Shift to bits 1-3
    ora 5,s             ; Combine with name table bit

    ; Store attribute byte
    sep #$20
    sta oam_low+3,x
    rep #$20

    ; Clean up stack
    pla
    pla
    pla

@invalid:
    rts
.endproc

; ============================================================================
; snes_sprite_hide - Hide a sprite
; Args: A = id
; ============================================================================
.proc snes_sprite_hide
    cmp #128
    bcs @invalid

    ; Calculate OAM index
    asl a
    asl a
    tax

    ; Set Y = 240 (off-screen)
    lda #240
    sep #$20
    sta oam_low+1,x
    rep #$20

@invalid:
    rts
.endproc

; ============================================================================
; snes_sprites_upload - Upload OAM buffer to PPU
; ============================================================================
.proc snes_sprites_upload
    ; Set OAM address to 0
    sep #$20
    stz OAMADDL
    stz OAMADDH
    rep #$20

    ; Upload low table (512 bytes)
    ldx #0
@upload_low:
    sep #$20
    lda oam_low,x
    sta OAMDATA
    rep #$20
    inx
    cpx #512
    bne @upload_low

    ; Upload high table (32 bytes)
    ldx #0
@upload_high:
    sep #$20
    lda oam_high,x
    sta OAMDATA
    rep #$20
    inx
    cpx #32
    bne @upload_high

    rts
.endproc

; ============================================================================
; snes_dma_vram - Upload data to VRAM using DMA
; Args: A = source address (low 16 bits), X = VRAM dest address, Y = size in bytes
; Note: Source must be in bank 0 (first 64KB)
; NOTE: Uses only stack for temps, preserves all DP locations
; ============================================================================
.proc snes_dma_vram
    ; Save params on stack
    pha                 ; source
    phx                 ; vram_dest
    phy                 ; size

    ; Set VRAM address
    sep #$20
    lda #$80
    sta $2115           ; VRAM increment mode
    rep #$20
    lda 3,s             ; vram_dest
    sta $2116           ; VRAM address

    ; Set up DMA channel 0
    sep #$20
    lda #$01            ; DMA mode: 2 regs write once
    sta $4300
    lda #$18            ; Dest: $2118 (VMDATAL)
    sta $4301
    rep #$20

    lda 5,s             ; source address
    sta $4302
    sep #$20
    lda #$00            ; Source bank (bank 0)
    sta $4304
    rep #$20

    lda 1,s             ; size
    sta $4305

    ; Start DMA
    sep #$20
    lda #$01            ; Enable DMA channel 0
    sta $420B
    rep #$20

    ; Clean up stack
    pla
    pla
    pla
    rts
.endproc

; ============================================================================
; snes_set_sprite_palette - Set up a basic sprite palette
; Sets palette 128 (first OBJ palette) with yellow color
; ============================================================================
.proc snes_set_sprite_palette
    ; Sprite palettes start at CGRAM address 128 (palette 0 for OBJ)
    sep #$20
    lda #128            ; OBJ palette 0 starts at CGRAM 128
    sta CGADD

    ; Color 0: transparent (black)
    stz CGDATA
    stz CGDATA

    ; Colors 1-14: unused (black)
    ldx #14
@fill_black:
    stz CGDATA
    stz CGDATA
    dex
    bne @fill_black

    ; Color 15: bright yellow ($03FF = R31, G31, B0)
    lda #$FF            ; Low byte (R=31, G low bits)
    sta CGDATA
    lda #$03            ; High byte (G high bits, B=0)
    sta CGDATA

    rep #$20
    rts
.endproc

; ============================================================================
; snes_load_sprite_tiles - Load the built-in sprite tiles to VRAM
; Uploads sprite_data to VRAM address 0
; ============================================================================
.export snes_load_sprite_tiles
.proc snes_load_sprite_tiles
    ; Upload sprite data to VRAM 0
    lda #sprite_data
    ldx #0              ; VRAM address 0
    ldy #32             ; 32 bytes (one 4bpp 8x8 tile)
    jsr snes_dma_vram
    rts
.endproc
