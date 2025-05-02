void test_shr()
{
    printf("test_shr\n");

    printf("    sign bit is set\n");
    reset_vm();

    regfile[R10] = 0x8000;
    regfile[R11] = 4;
    shr(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0x0800);

    printf("    sign bit is not set\n");
    reset_vm();

    regfile[R10] = 0x7fff;
    regfile[R11] = 4;
    shr(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0x07ff);
}
