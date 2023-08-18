section data
varw1: 25
varw2: 41
varw3: 0

varb1: .byte 9
varb2: .byte 7
varb3: .byte 0

section text
_start:
    push varw1 ld
    push varw2 ld
    add
    push varw3 st
    push varw3 ld

    push varb1 ldb
    push varb2 ldb
    addb
    push varb3 stb
    push varb3 ldb

    halt
