#define SYS_write 1

.global io.putc, io.puts;, io.putn
.extern string.len;, strconv.itoa

; a ->
io.putc:
    rsp push 1 sub rspset
    rsp stb

    rsp push 1
    syscall SYS_write

    rsp push 1 add rspset
    ret

; ptr8 ->
io.puts:
    call string.len
    syscall SYS_write
    ret

; a ->
;io.putn:
;    rsp push 6 sub rspset ; buf
;
;    rsp
;    call strconv.itoa
;    syscall SYS_write
;
;    rsp push 6 add rspset
;    ret
