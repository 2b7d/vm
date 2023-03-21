message: "hello, world" 10 0

_start:
    push message
    call strlen
    syscall 1

    halt

; ptr -> ptr n1
strlen:
    push 0
strlen_loop:
    over
    over
    add
    ld

    push 0
    eq
    jmpif strlen_done

    inc
    jmp strlen_loop
strlen_done:
    ret
