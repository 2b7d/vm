void test_call()
{
    printf("test_call\n");
    reset_vm();

    call(69);

    pc = 69;
    movi(1, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 1);
}
