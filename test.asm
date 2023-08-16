// this is comment

pushb 7
pushb 10
gtb
push do_add
cjmp

pushb 6
pushb 4
subb
push exit
jmp

do_add:
    pushb 4
    pushb 6
    addb

exit:
    halt // comment
