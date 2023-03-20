_start:
    push 69
    call itoa

    push 420
    call itoa

    push 8
    call itoa

    halt

buffer: 0 0 0 0 0 0 0

; n1 ->
itoa:
    dup
    retpush
    push 1
    swap
itoa_loop:
    push 10 mod
    push 48
    add

    retpop
    swap
    retpush

    push 10 div
    dup

    push 0
    eq

    ifjmp itoa_done

    swap
    inc swap
    dup
    retpush
    jmp itoa_loop
itoa_done:
    drop
    push 0
itoa_reverse:
    dup
    retpop
    swap
    push buffer add st

    inc
    over
    over

    swap
    lt
    ifjmp itoa_reverse

    push buffer add
    push 10
    swap
    st

    push 1 add
    push buffer swap
    syscall 1
    ret
