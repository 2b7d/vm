#define LF 10
#define NULL 0
#define SYS_write 1

.global _start
.extern strlen

message: "hello, world" .bytes LF NULL

_start:
    push message
    call strlen
    syscall SYS_write

    halt

