void test_shrai()
{
    printf("test_shrai\n");

    printf("    sign bit is set\n");
    reset_vm();

    regfile[R10] = 0x8000;
    shrai(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xf800);

    printf("    sign bit is not set\n");
    reset_vm();

    regfile[R10] = 0x7fff;
    shrai(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0x07ff);
}
