void test_or()
{
    printf("test_or\n");
    reset_vm();

    regfile[R10] = 0xff00;
    regfile[R11] = 0x00ff;
    or(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xffff);
}
