.global strlen

; ptrb -> ptrb n1
strlen:
    push 0
strlen_loop:
    over over add ld8

    push8 0 eq8
    jmpif strlen_done

    inc
    jmp strlen_loop
strlen_done:
    ret
