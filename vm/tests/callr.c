void test_callr()
{
    printf("test_callr\n");
    reset_vm();

    regfile[R10] = 69;
    callr(R10);

    pc = 69;
    movi(1, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 1);
}
