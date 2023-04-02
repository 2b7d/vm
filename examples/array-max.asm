arr: 6 8 1 69 4
arr_len: 10
max: 0

.global _start
_start:
    push arr ld
    push max st

    push 2
loop:
    dup
    push arr
    add
    ld

    dup
    push max ld
    gt

    jmpif new_max

    drop

inc_counter:
    push 2 add
    dup
    push arr_len ld
    lt
    jmpif loop

    halt

new_max:
    push max st
    jmp inc_counter
