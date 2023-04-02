_start:
    push 20 rspush ; +2 len
    push 0 rspush  ; +1 arr

    brk rsp push 2 add ld add brkset
    rsp push 1 add st

    push 0
loop:
    dup
    rscopy
    over
    add
    st
    inc
    dup
    rsp push 2 add ld
    lt
    jmpif loop

    drop
    rscopy brkset
    drop rsdrop rsdrop

    halt
