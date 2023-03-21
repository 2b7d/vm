array: 3 5 2 1 4 6
array_len: 6
sum: 0

_start:
    push 0
loop:
    dup
    push array
    add
    ld

    push sum ld
    add
    push sum st

    inc
    dup
    push array_len ld
    lt
    jmpif loop

    halt
