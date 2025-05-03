void test_shlb()
{
    printf("test_shlb\n");
    reset_vm();

    regfile[R10] = 0xabff;
    regfile[R11] = 4;
    shlb(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xabf0);
}
