;#define LF 10
;#define NULL 0
;
;.global _start
;.extern io.puts
;
;message: "hello, world" .bytes LF NULL
;
;_start:
;    push message
;    call io.puts
;
;    halt


#define LF 10
#define NULL 0
#define SYS_write 1

.global _start

message: "hello, world" .bytes LF NULL

_start:
    push message

    push 0
len_loop:
    over over add ldb

    pushb NULL eq
    jmpif len_done

    inc
    jmp len_loop
len_done:
    syscall SYS_write

    halt
