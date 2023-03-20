message: "hello, world" 10
message_len: 13

_start:
    push message
    push message_len ld
    syscall 1

    halt
