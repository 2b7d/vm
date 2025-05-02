void test_shra()
{
    printf("test_shra\n");

    printf("    sign bit is set\n");
    reset_vm();

    regfile[R10] = 0x8000;
    regfile[R11] = 4;
    shra(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xf800);

    printf("    sign bit is not set\n");
    reset_vm();

    regfile[R10] = 0x7fff;
    regfile[R11] = 4;
    shra(R11, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0x07ff);
}
