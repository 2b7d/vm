section data
_0: .byte 0
_1: .byte 1
_2: .byte 2

section text
_start:
    push _1
    ctb
    push _2
    ctb
    addb
    ctw
    halt
