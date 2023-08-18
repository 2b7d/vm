section data
msg: .byte "hello, world", 10
msg_len: 13
index: 0
char: .byte 0

section text
_start:
    push 1
    push msg
    push msg_len ld
    push 1
    syscall

loop:
    push index ld
    push msg_len ld
    eq
    push done
    cjmp

    push index ld
    push msg
    add
    ldb
    push char stb

    push 1
    push char
    push 1
    push 1
    syscall

    push index ld
    push 1
    add
    push index st
    push loop
    jmp
done:
    halt
