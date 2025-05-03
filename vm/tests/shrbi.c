void test_shrbi()
{
    printf("test_shrbi\n");

    printf("    sign bit is set\n");
    reset_vm();

    regfile[R10] = 0xab80;
    shrbi(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xab08);

    printf("    sign bit is not set\n");
    reset_vm();

    regfile[R10] = 0xab7f;
    shrbi(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xab07);
}
