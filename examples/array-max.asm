arr: 6 8 1 69 4
arr_len: 5
max: 0

_start:
    push arr ld
    push max st

    push 1
loop:
    dup
    push arr
    add
    ld

    dup
    push max ld
    gt

    ifjmp new_max

    drop

inc_counter:
    inc
    dup
    push arr_len ld
    lt
    ifjmp loop

    halt

new_max:
    push max st
    jmp inc_counter
