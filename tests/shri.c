void test_shri()
{
    printf("test_shri\n");

    printf("    sign bit is set\n");
    reset_vm();

    regfile[R10] = 0x8000;
    shri(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0x0800);

    printf("    sign bit is not set\n");
    reset_vm();

    regfile[R10] = 0x7fff;
    shri(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0x07ff);
}
