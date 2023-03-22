#define SYS_write 1
#define NULL 0
#define LF 10

_start:
    rsp push 7 sub rspset ; +1 buf[7]

    rsp push 1 add
    push 69
    swap
    call itoa
    dup
    call strlen
    syscall SYS_write

    push 420
    swap
    call itoa
    dup
    call strlen
    syscall SYS_write

    push 8
    swap
    call itoa
    call strlen
    syscall SYS_write

    halt

; n1 ptr -> ptr
itoa:
    rspush        ; +3 buf
    push 0 rspush ; +2 count
    rspush        ; +1 val
itoa_translate_int:
    rscopy push 10 mod push 48 add
    rspop push 10 div rspush
    rsp push 2 add dup ld inc swap st
    rscopy push 0 eq not jmpif itoa_translate_int

    push 0
itoa_copyto_buf:
    swap over
    rsp push 3 add ld add st

    inc dup
    rsp push 2 add ld
    lt jmpif itoa_copyto_buf

    rsp push 3 add ld
    over over
    add
    push LF swap st

    swap inc swap
    add
    push NULL swap st

    rsdrop rsdrop rspop
ret

; ptr -> ptr n1
strlen:
    push 0
strlen_loop:
    over over add ld
    push NULL eq
    jmpif strlen_done

    inc
    jmp strlen_loop
strlen_done:
ret
