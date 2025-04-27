void test_jabs()
{
    printf("test_jabs\n");
    reset_vm();

    jabs(69);
    halt();

    pc = 69;
    movi(8, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 8);
}
