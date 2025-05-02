void test_shl()
{
    printf("test_shl\n");
    reset_vm();

    regfile[R10] = 0xffff;
    regfile[R11] = 4;
    shl(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xfff0);
}
