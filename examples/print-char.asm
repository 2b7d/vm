#define LF 10

.global _start
.extern io.putc

_start:
    push8 "6"
    call io.putc

    push8 "9"
    call io.putc

    push8 LF
    call io.putc

    halt
