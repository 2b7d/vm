#define NULL 0

.global string.len

; ptr8 -> ptr8 a
string.len:
    push 0
len_loop:
    over over add ld8

    push8 NULL eq8
    jmpif len_done

    inc
    jmp len_loop
len_done:
    ret
