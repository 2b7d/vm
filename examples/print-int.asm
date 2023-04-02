#define LF 10
#define NULL 0

.global _start
.extern io.putn, io.puts, io.putc

message: "this is test for printing numbers" .bytes LF NULL

_start:
    push message
    call io.puts

    push 69
    call io.putn
    push8 LF
    call io.putc

    push 420
    call io.putn
    push8 LF
    call io.putc

    halt
