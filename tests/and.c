void test_and()
{
    printf("test_and\n");
    reset_vm();

    regfile[R10] = 0xabcd;
    regfile[R11] = 0x00ff;
    and(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0x00cd);
}
