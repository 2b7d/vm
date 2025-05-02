void test_shrabi()
{
    printf("test_shrabi\n");

    printf("    sign bit is set\n");
    reset_vm();

    regfile[R10] = 0xab80;
    shrabi(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xabf8);

    printf("    sign bit is not set\n");
    reset_vm();

    regfile[R10] = 0xab7f;
    regfile[R11] = 4;
    shrabi(4, R10);
    halt();

    pc = 0;
    vm_start();

    assert(regfile[R10] == 0xab07);
}
