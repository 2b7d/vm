void test_shrab()
{
    printf("test_shrab\n");

    printf("    sign bit is set\n");
    reset_vm();

    regfile[R10] = 0xab80;
    regfile[R11] = 4;
    shrab(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xabf8);

    printf("    sign bit is not set\n");
    reset_vm();

    regfile[R10] = 0xab7f;
    regfile[R11] = 4;
    shrab(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xab07);
}
