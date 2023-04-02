#define SYS_write 1

.global io.putc, io.puts, io.putn
.extern string.len, strconv.itoa

; a8 ->
io.putc:
    rspush8 ; +1 a8

    rsp push 1 add push 1
    syscall SYS_write

    rsdrop8
    ret

; ptr8 ->
io.puts:
    call string.len
    syscall SYS_write
    ret

; a ->
io.putn:
    rsp push 6 sub rspset ; +1 buf

    rsp push 1 add
    call strconv.itoa
    syscall SYS_write

    rsp push 6 add rspset
    ret
