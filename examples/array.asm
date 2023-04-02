array: 3 5 2 1 4 6
array_len: 12
sum: 0

.global _start
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

    push 2 add
    dup
    push array_len ld
    lt
    jmpif loop

    halt
