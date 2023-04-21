#define LF 10

.global _start
.extern io.putc

_start:
    pushb "6"
    call io.putc

    pushb "9"
    call io.putc

    pushb LF
    call io.putc

    halt
