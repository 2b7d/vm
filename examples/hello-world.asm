#define LF 10
#define NULL 0

.global _start
.extern io.puts

message: "hello, world" .bytes LF NULL

_start:
    push message
    call io.puts

    halt
