; find sum of multiples of 3 or 5

range: 500
sum: 0

_start:
    push 1 ; num

loop:
    dup
    push 3
    mod
    push 0
    eq ; num % 3 == 0

    over

    push 5
    mod
    push 0
    eq ; num % 5 == 0

    or
    not
    jmpif skip_add

    dup
    push sum ld
    add
    push sum st
skip_add:
    inc ; num++
    dup
    push range ld
    lt
    jmpif loop ; while (num < range)

    halt
