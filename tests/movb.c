void test_movb()
{
    printf("test_movb\n");
    reset_vm();

    regfile[R10] = 0xff80;
    movb(R10, R11);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R11] == 0x0080);
}
