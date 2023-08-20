#define SYS_write 1
#define STDOUT    1

#define push_arg  (x) push _fp ld push x sub ld
#define  pop_arg  (x) push _fp ld push x sub st
#define push_loc  (x) push _fp ld push x add ld
#define  pop_loc  (x) push _fp ld push x add st
#define push_ret      push _rp ld

msg: .byte "hello, world", 10, 0

strlen: // *b-4 buf -> w length
    push 0 // w+2 index
strlen_loop:
    push_loc 2
    push_arg 4
    add
    ldb

    pushb 0
    eqb
    push strlen_done
    cjmp

    push_loc 2
    push 1
    add
    pop_loc 2

    push strlen_loop
    jmp
strlen_done:
    push_loc 2
    ret

print: // *b-4 buf -> void
    push_arg 4
    call strlen
    drop

    push STDOUT
    push_arg 4
    push_ret
    push SYS_write
    syscall

    push 0
    ret

_start:
    push msg
    call print
    drop
    halt
