_start:
    push 2
    push 4
    call sum
    push _sp ld push 4 sub push _sp st
    push _rp ld
    halt

sum: // w1, w2 -> w
    push _fp ld push 4 sub ld
    call inc
    push _sp ld push 2 sub push _sp st // art: should add drop/dropb opcode?
    push _rp ld
    push _fp ld push 6 sub ld
    add
    ret

inc: // w1 -> w
    push _fp ld push 4 sub ld
    push 1
    add
    ret
