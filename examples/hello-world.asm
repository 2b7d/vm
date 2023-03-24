#define LF 10
#define NULL 0
#define SYS_write 1

message: "hello, world" LF NULL

_start:
    push message
    call strlen
    syscall SYS_write

    halt

; ptrb -> ptrb n1
strlen:
    push 0
strlen_loop:
    over over add ld8

    push8 0 eq8
    jmpif strlen_done

    inc
    jmp strlen_loop
strlen_done:
ret
