a: 3
b: 4
product: 0

_start:
    push 0 ; counter
loop:
    push product ld
    push a ld
    add
    push product st ; product = product + a

    push 1
    add ; counter++

    dup
    push b ld
    lt
    ifjmp loop ; while (counter < b)

    halt
