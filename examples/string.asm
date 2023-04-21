#define NULL 0

.global string.len

; ptr8 -> ptr8 a
string.len:
    push 0
len_loop:
    over over add ldb

    pushb NULL eq
    jmpif len_done

    inc
    jmp len_loop
len_done:
    ret
