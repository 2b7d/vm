void test_ret()
{
    printf("test_ret\n");
    reset_vm();

    call(69);
    movi(2, R10);
    halt();

    pc = 69;
    movi(1, R10);
    ret();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 2);
}
