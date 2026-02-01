;; Parallax Scroller Graphics Data
;; Assembled with ca65 --cpu 65816
;;
;; SNES colors are BGR555 format:
;;   Bits 0-4: Red (0-31)
;;   Bits 5-9: Green (0-31)
;;   Bits 10-14: Blue (0-31)

.segment "RODATA"

;; ============================================================
;; Palette for Parallax Demo (BG1 and BG2 share first 32 colors)
;; ============================================================

.export parallax_palette
.export parallax_palette_end

parallax_palette:
    ; Color 0: Sky blue (background)
    ; RGB(8, 16, 28) = 8 | (16<<5) | (28<<10) = $7208
    .word $7208

    ; BG1 Colors (Far mountains - darker, blue-ish)
    ; Color 1: Dark purple-blue mountain
    ; RGB(8, 4, 16) = $4088
    .word $4088
    ; Color 2: Medium mountain
    ; RGB(12, 8, 18) = $490C
    .word $490C
    ; Color 3: Light mountain highlight
    ; RGB(16, 12, 20) = $5210
    .word $5210

    ; BG2 Colors (Near hills - greener)
    ; Color 4: Dark green hill
    ; RGB(4, 12, 6) = $1984
    .word $1984
    ; Color 5: Medium green
    ; RGB(8, 18, 8) = $2248
    .word $2248
    ; Color 6: Light green highlight
    ; RGB(12, 24, 12) = $330C
    .word $330C
    ; Color 7: Grass green
    ; RGB(8, 20, 4) = $1288
    .word $1288

    ; Colors 8-15: Extended palette (black for now)
    .word $0000, $0000, $0000, $0000
    .word $0000, $0000, $0000, $0000

parallax_palette_end:

;; ============================================================
;; Sprite Palette (colors 128-143, palette 0)
;; ============================================================

.export player_palette
.export player_palette_end

player_palette:
    .word $0000         ; 0: Transparent
    .word $7FFF         ; 1: White (outline)
    .word $03E0         ; 2: Bright green (body)
    .word $02A0         ; 3: Dark green (shadow)
    .word $001F         ; 4: Red (accent)
    .word $0000, $0000, $0000  ; 5-7
    .word $0000, $0000, $0000, $0000  ; 8-11
    .word $0000, $0000, $0000  ; 12-14
    .word $03FF         ; 15: Bright yellow (smiley face color)

player_palette_end:

;; ============================================================
;; BG1 Tiles: Far Mountains - Rolling Hills (2bpp, 16 bytes each)
;; Smooth curved shapes for gentle rolling effect
;; ============================================================

.export mountain_tiles
.export mountain_tiles_end

mountain_tiles:
    ; Tile 0: Empty (transparent)
    .byte $00, $00, $00, $00, $00, $00, $00, $00
    .byte $00, $00, $00, $00, $00, $00, $00, $00

    ; Tile 1: Solid fill
    .byte $FF, $00, $FF, $00, $FF, $00, $FF, $00
    .byte $FF, $00, $FF, $00, $FF, $00, $FF, $00

    ; Tile 2: Gentle curve top (wide dome)
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $18, $00  ; Row 4: ...XX...
    .byte $3C, $00  ; Row 5: ..XXXX..
    .byte $7E, $00  ; Row 6: .XXXXXX.
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 3: Gentle rise from right (connects right side)
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $00, $00  ; Row 4
    .byte $03, $00  ; Row 5: ......XX
    .byte $0F, $00  ; Row 6: ....XXXX
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 4: Gentle rise from left (connects left side)
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $00, $00  ; Row 4
    .byte $C0, $00  ; Row 5: XX......
    .byte $F0, $00  ; Row 6: XXXX....
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 5: Very gentle rise right
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $00, $00  ; Row 4
    .byte $00, $00  ; Row 5
    .byte $07, $00  ; Row 6: .....XXX
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 6: Very gentle rise left
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $00, $00  ; Row 4
    .byte $00, $00  ; Row 5
    .byte $E0, $00  ; Row 6: XXX.....
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 7: Half fill (bottom half solid)
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $FF, $00  ; Row 4: XXXXXXXX
    .byte $FF, $00  ; Row 5
    .byte $FF, $00  ; Row 6
    .byte $FF, $00  ; Row 7

mountain_tiles_end:

;; ============================================================
;; BG2 Tiles: Near Hills - Rolling Hills (2bpp, 16 bytes each)
;; Same smooth curves as mountains for consistent look
;; ============================================================

.export hill_tiles
.export hill_tiles_end

hill_tiles:
    ; Tile 0: Empty (transparent)
    .byte $00, $00, $00, $00, $00, $00, $00, $00
    .byte $00, $00, $00, $00, $00, $00, $00, $00

    ; Tile 1: Solid fill
    .byte $FF, $00, $FF, $00, $FF, $00, $FF, $00
    .byte $FF, $00, $FF, $00, $FF, $00, $FF, $00

    ; Tile 2: Gentle curve top (wide dome)
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $18, $00  ; Row 4: ...XX...
    .byte $3C, $00  ; Row 5: ..XXXX..
    .byte $7E, $00  ; Row 6: .XXXXXX.
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 3: Gentle rise from right
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $00, $00  ; Row 4
    .byte $03, $00  ; Row 5: ......XX
    .byte $0F, $00  ; Row 6: ....XXXX
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 4: Gentle rise from left
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $00, $00  ; Row 4
    .byte $C0, $00  ; Row 5: XX......
    .byte $F0, $00  ; Row 6: XXXX....
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 5: Very gentle rise right
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $00, $00  ; Row 4
    .byte $00, $00  ; Row 5
    .byte $07, $00  ; Row 6: .....XXX
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 6: Very gentle rise left
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $00, $00  ; Row 4
    .byte $00, $00  ; Row 5
    .byte $E0, $00  ; Row 6: XXX.....
    .byte $FF, $00  ; Row 7: XXXXXXXX

    ; Tile 7: Half fill (bottom half solid)
    .byte $00, $00  ; Row 0
    .byte $00, $00  ; Row 1
    .byte $00, $00  ; Row 2
    .byte $00, $00  ; Row 3
    .byte $FF, $00  ; Row 4: XXXXXXXX
    .byte $FF, $00  ; Row 5
    .byte $FF, $00  ; Row 6
    .byte $FF, $00  ; Row 7

hill_tiles_end:

;; ============================================================
;; Player Sprite (8x8, 4bpp, 32 bytes)
;; ============================================================

.export player_sprite
.export player_sprite_end

player_sprite:
    ; Simple character - 8x8 4bpp
    ; Row-interleaved bitplanes: BP0, BP1 for row 0, then BP2, BP3 for row 0, etc.

    ; Rows 0-7, bitplanes 0-1 (interleaved by row)
    .byte $3C, $3C  ; Row 0: ..XXXX..
    .byte $7E, $42  ; Row 1: .XXXXXX. (outline + fill)
    .byte $FF, $81  ; Row 2: XXXXXXXX
    .byte $FF, $A5  ; Row 3: XX.XX.XX (eyes)
    .byte $FF, $81  ; Row 4: XXXXXXXX
    .byte $FF, $99  ; Row 5: X.XXXX.X (mouth)
    .byte $7E, $42  ; Row 6: .XXXXXX.
    .byte $3C, $3C  ; Row 7: ..XXXX..

    ; Rows 0-7, bitplanes 2-3 (interleaved by row)
    .byte $00, $00  ; Row 0
    .byte $3C, $00  ; Row 1 (green fill)
    .byte $7E, $00  ; Row 2
    .byte $5A, $00  ; Row 3 (not eyes)
    .byte $7E, $00  ; Row 4
    .byte $66, $00  ; Row 5 (not mouth)
    .byte $3C, $00  ; Row 6
    .byte $00, $00  ; Row 7

player_sprite_end:

;; ============================================================
;; BG1 Tilemap: Far Rolling Hills (32x32 tiles = 2KB)
;; Gentle undulating hills in the distance
;; Tiles: 0=empty, 1=solid, 2=dome, 3=rise-R, 4=rise-L, 5=gentle-R, 6=gentle-L, 7=half
;; ============================================================

.export mountain_tilemap
.export mountain_tilemap_end

mountain_tilemap:
    ; Rows 0-17: Sky (empty)
    .repeat 18
        .repeat 32
            .word $0000
        .endrep
    .endrep

    ; Row 18: Distant hill tops - gentle rolling curves
    .word $0000, $0000, $0005, $0007, $0007, $0006, $0000, $0000  ; 0-7
    .word $0000, $0005, $0007, $0007, $0007, $0006, $0000, $0000  ; 8-15
    .word $0005, $0007, $0007, $0006, $0000, $0000, $0005, $0007  ; 16-23
    .word $0007, $0007, $0006, $0000, $0000, $0005, $0007, $0006  ; 24-31

    ; Row 19: Continuing curves
    .word $0000, $0003, $0001, $0001, $0001, $0001, $0004, $0000  ; 0-7
    .word $0003, $0001, $0001, $0001, $0001, $0001, $0004, $0005  ; 8-15
    .word $0001, $0001, $0001, $0001, $0004, $0003, $0001, $0001  ; 16-23
    .word $0001, $0001, $0001, $0004, $0003, $0001, $0001, $0001  ; 24-31

    ; Rows 20-31: Solid fill
    .repeat 12
        .repeat 32
            .word $0001
        .endrep
    .endrep

mountain_tilemap_end:

;; ============================================================
;; BG2 Tilemap: Near Rolling Hills (32x32 tiles = 2KB)
;; Foreground hills with gentle curves, priority bit for layering
;; Tiles: 0=empty, 1=solid, 2=dome, 3=rise-R, 4=rise-L, 5=gentle-R, 6=gentle-L, 7=half
;; ============================================================

.export hill_tilemap
.export hill_tilemap_end

hill_tilemap:
    ; Rows 0-20: Sky/transparent (mountains visible above)
    .repeat 21
        .repeat 32
            .word $0000
        .endrep
    .endrep

    ; Row 21: Near hill tops - gentle curves with priority ($2000)
    .word $0000, $2005, $2007, $2007, $2006, $0000, $0000, $0000  ; 0-7
    .word $2005, $2007, $2006, $0000, $0000, $2005, $2007, $2007  ; 8-15
    .word $2007, $2006, $0000, $0000, $2005, $2007, $2007, $2006  ; 16-23
    .word $0000, $0000, $0000, $2005, $2007, $2007, $2007, $2006  ; 24-31

    ; Row 22: Continuing curves
    .word $2003, $2001, $2001, $2001, $2001, $2004, $0000, $2003  ; 0-7
    .word $2001, $2001, $2001, $2004, $2003, $2001, $2001, $2001  ; 8-15
    .word $2001, $2001, $2004, $2003, $2001, $2001, $2001, $2001  ; 16-23
    .word $2004, $0000, $2003, $2001, $2001, $2001, $2001, $2001  ; 24-31

    ; Rows 23-31: Solid fill with priority
    .repeat 9
        .repeat 32
            .word $2001
        .endrep
    .endrep

hill_tilemap_end:
