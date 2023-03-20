message: "hello, world"
len: 12

_start:
    push 0
loop:
    dup
    push message
    add
    ld

    swap
    inc

    dup
    push len ld
    lt
    ifjmp loop

    drop

    halt
