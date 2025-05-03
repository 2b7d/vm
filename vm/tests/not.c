void test_not()
{
    printf("test_not\n");
    reset_vm();

    regfile[R10] = 0xff00;
    not(R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0x00ff);
}
