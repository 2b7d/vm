#define NULL 0

.global strconv.itoa

; a ptr8 -> ptr8 b
strconv.itoa:
    rspush        ; +7 buf
    push 0 rspush ; +5 count
    push 0 rspush ; +3 index
    rspush        ; +1 val

    push8 NULL
itoa_parse_int:
    rscopy push 10
    mod wtb
    push8 "0"
    add8

    rspop push 10 div rspush
    rsp push 5 add
    dup ld inc swap st

    rscopy push 0 eq
    not jmpif itoa_parse_int

itoa_copy:
    rsp push 3 add ld
    rsp push 7 add ld
    add
    st8

    rsp push 3 add ld
    inc dup
    rsp push 3 add st

    rsp push 5 add ld
    lt jmpif itoa_copy

    rsp push 3 add ld
    rsp push 7 add ld
    add
    st8

    rsdrop rsdrop
    rspop rspop swap

    ret
