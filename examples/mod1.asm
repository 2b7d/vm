.global _start
.extern mod2_inc
.extern mod3_msg, mod3_msglen

_start:
    push 7
    call mod2_inc
    drop
    push _rp ld

    push 1
    push mod3_msg
    push mod3_msglen
    push 1
    syscall
    drop

    halt
